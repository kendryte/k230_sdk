/*
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Technology (China) Co., Ltd.
 *
 *            (C) COPYRIGHT 2021-2021 Arm Technology (China) Co., Ltd.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Technology (China) Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/delay.h>
#include "mvx-v4l2-controls.h"
#include "mvx_bitops.h"
#include "mvx_firmware.h"
#include "mvx_firmware_cache.h"
#include "mvx_session.h"
#include "mvx_seq.h"

/****************************************************************************
 * Private variables
 ****************************************************************************/

static int session_watchdog_timeout = 30000;
module_param(session_watchdog_timeout, int, 0660);

static int fw_watchdog_timeout;
module_param(fw_watchdog_timeout, int, 0660);

/****************************************************************************
 * Private functions
 ****************************************************************************/

static void watchdog_start(struct mvx_session *session,
               unsigned int timeout_ms)
{
    int ret;

    if (session->error != 0)
        return;

    MVX_SESSION_DEBUG(session, "Watchdog start. timeout_ms=%u.",
              timeout_ms);

    ret = mod_timer(&session->watchdog_timer,
            jiffies + msecs_to_jiffies(timeout_ms));
    if (ret != 0) {
        MVX_SESSION_WARN(session, "Failed to start watchdog. ret=%d",
                 ret);
        return;
    }

    session->watchdog_count = 0;

    kref_get(&session->isession.kref);
}

static void watchdog_stop(struct mvx_session *session)
{
    int ret;

    ret = del_timer_sync(&session->watchdog_timer);

    /* ret: 0=watchdog expired, 1=watchdog still running */
    MVX_SESSION_DEBUG(session, "Watchdog stop. ret=%d", ret);

    /* Decrement the kref if the watchdog was still running. */
    if (ret != 0)
        kref_put(&session->isession.kref, session->isession.release);
}

static void watchdog_update(struct mvx_session *session,
                unsigned int timeout_ms)
{
    int ret;

    ret = mod_timer_pending(&session->watchdog_timer,
                jiffies + msecs_to_jiffies(timeout_ms));

    session->watchdog_count = 0;

    /* ret: 0=no restart, 1=restarted */
    MVX_SESSION_DEBUG(session, "Watchdog update. ret=%d, timeout_ms=%u.",
              ret, timeout_ms);
}

static bool is_fw_loaded(struct mvx_session *session)
{
    return (IS_ERR_OR_NULL(session->fw_bin) == false);
}

static void print_debug(struct mvx_session *session)
{
    MVX_SESSION_INFO(session, "Print debug.");

    if (session->csession != NULL)
        session->client_ops->print_debug(session->csession);

    if (is_fw_loaded(session))
        session->fw.ops.print_debug(&session->fw);
}

static void send_event_error(struct mvx_session *session,
                 long error)
{
    session->error = error;
    wake_up(&session->waitq);
    session->event(session, MVX_SESSION_EVENT_ERROR,
               (void *)session->error);
}

static void session_unregister(struct mvx_session *session)
{
    if (!IS_ERR_OR_NULL(session->csession)) {
        session->client_ops->unregister_session(session->csession);
        session->csession = NULL;
    }
}

static void release_fw_bin(struct mvx_session *session)
{
    if (is_fw_loaded(session) != false) {
        MVX_SESSION_INFO(session, "Release firmware binary.");

        mvx_fw_destruct(&session->fw);
        mvx_fw_cache_put(session->cache, session->fw_bin);
        session->fw_bin = NULL;
    }

    watchdog_stop(session);
    session_unregister(session);
}

static struct mvx_session *kref_to_session(struct kref *kref)
{
    return container_of(kref, struct mvx_session, isession.kref);
}

static void session_destructor(struct kref *kref)
{
    struct mvx_session *session = kref_to_session(kref);

    session->destructor(session);
}

static const char *state_to_string(enum mvx_fw_state state)
{
    switch (state) {
    case MVX_FW_STATE_STOPPED:
        return "Stopped";
    case MVX_FW_STATE_RUNNING:
        return "Running";
    default:
        return "Unknown";
    }
}

static enum mvx_direction get_bitstream_port(struct mvx_session *session)
{
    if (mvx_is_bitstream(session->port[MVX_DIR_INPUT].format) &&
        mvx_is_frame(session->port[MVX_DIR_OUTPUT].format))
        return MVX_DIR_INPUT;
    else if (mvx_is_frame(session->port[MVX_DIR_INPUT].format) &&
         mvx_is_bitstream(session->port[MVX_DIR_OUTPUT].format))
        return MVX_DIR_OUTPUT;

    return MVX_DIR_MAX;
}

static bool is_stream_on(struct mvx_session *session)
{
    return session->port[MVX_DIR_INPUT].stream_on &&
           session->port[MVX_DIR_OUTPUT].stream_on;
}

/**
 * wait_pending() - Wait for procedure to finish.
 *
 * Wait for the number of pending firmware messages to reach 0, or for an error
 * to happen.
 *
 * Return: 0 on success, else error code.
 */
static int wait_pending(struct mvx_session *session)
{
    int ret = 0;

    while (is_fw_loaded(session) != false &&
           session->fw.msg_pending > 0 &&
           session->error == 0) {
        mutex_unlock(session->isession.mutex);

        ret = wait_event_timeout(
            session->waitq,
            is_fw_loaded(session) == false ||
            session->fw.msg_pending == 0 ||
            session->error != 0,
            msecs_to_jiffies(10000));

        if (ret < 0)
            goto lock_mutex;

        if (ret == 0) {
            send_event_error(session, -ETIME);
            ret = -ETIME;
            goto lock_mutex;
        }

        mutex_lock(session->isession.mutex);
    }

    return session->error;

lock_mutex:
    mutex_lock(session->isession.mutex);

    if (ret < 0)
        MVX_SESSION_WARN(session,
                 "Wait pending returned error. ret=%d, error=%d, msg_pending=%d.",
                 ret, session->error, session->fw.msg_pending);

    return ret;
}

static int send_irq(struct mvx_session *session)
{
    if (IS_ERR_OR_NULL(session->csession))
        return -EINVAL;

    return session->client_ops->send_irq(session->csession);
}

/**
 * switch_in() - Request the client device to switch in the session.
 *
 * Return: 0 on success, else error code.
 */
static int switch_in(struct mvx_session *session)
{
    int ret;

    session->idle_count = 0;

    if (session->switched_in != false)
        return 0;

    MVX_SESSION_INFO(session, "Switch in.");

    ret = session->client_ops->switch_in(session->csession);
    if (ret != 0) {
        MVX_SESSION_WARN(session, "Failed to switch in session.");
        send_event_error(session, ret);
        return ret;
    }

    session->switched_in = true;

    return 0;
}

/**
 * fw_send_msg() - Send firmware message and signal IRQ.
 *
 * Return: 0 on success, else error code.
 */
static int fw_send_msg(struct mvx_session *session,
               struct mvx_fw_msg *msg)
{
    int ret;

    if (session->error != 0)
        return session->error;

    ret = session->fw.ops.put_message(&session->fw, msg);
    if (ret != 0) {
        MVX_SESSION_WARN(session,
                 "Failed to queue firmware message.");
        goto send_error;
    }

    ret = send_irq(session);
    if (ret != 0) {
        MVX_SESSION_WARN(session, "Failed to send irq.");
        goto send_error;
    }

    return switch_in(session);

send_error:
    send_event_error(session, ret);
    return ret;
}

static int fw_send_msg_simple(struct mvx_session *session,
                  enum mvx_fw_code code,
                  const char *str)
{
    struct mvx_fw_msg msg = { .code = code };

    MVX_SESSION_INFO(session, "Firmware req: %s.", str);

    return fw_send_msg(session, &msg);
}

static int fw_flush(struct mvx_session *session,
            enum mvx_direction dir)
{
    struct mvx_fw_msg msg = { .code = MVX_FW_CODE_FLUSH, .flush.dir = dir };
    int ret;

    MVX_SESSION_INFO(session, "Firmware req: Flush. dir=%d.", dir);

    ret = fw_send_msg(session, &msg);
    if (ret != 0)
        return ret;

    session->port[dir].is_flushing = true;

    return 0;
}

static int fw_state_change(struct mvx_session *session,
               enum mvx_fw_state state)
{
    struct mvx_fw_msg msg = {
        .code  = MVX_FW_CODE_STATE_CHANGE,
        .state = state
    };
    int ret = 0;

    if (state != session->fw_state) {
        MVX_SESSION_INFO(session,
                 "Firmware req: State change. current=%d, new=%d.",
                 session->fw_state, state);
        ret = fw_send_msg(session, &msg);
    }

    return ret;
}

static int fw_job(struct mvx_session *session,
          unsigned int frames)
{
    struct mvx_fw_msg msg = {
        .code       = MVX_FW_CODE_JOB,
        .job.cores  = session->isession.ncores,
        .job.frames = frames
    };

    MVX_SESSION_INFO(session, "Firmware req: Job. frames=%u.", frames);

    return fw_send_msg(session, &msg);
}

static int fw_switch_out(struct mvx_session *session)
{
    unsigned int idle_count = session->idle_count;
    int ret;

    ret = fw_send_msg_simple(session, MVX_FW_CODE_SWITCH_OUT,
                 "Switch out");

    /*
     * Restore idle count. Switch out is the only message where we do not
     * want to reset the idle counter.
     */
    session->idle_count = idle_count;

    return ret;
}

static int fw_ping(struct mvx_session *session)
{
    return fw_send_msg_simple(session, MVX_FW_CODE_PING, "Ping");
}

static int fw_dump(struct mvx_session *session)
{
    return fw_send_msg_simple(session, MVX_FW_CODE_DUMP, "Dump");
}

static int fw_set_debug(struct mvx_session *session, uint32_t debug_level)
{
    struct mvx_fw_msg msg = {
        .code       = MVX_FW_CODE_DEBUG,
        .arg = debug_level
    };

    MVX_SESSION_INFO(session, "Firmware req: Set debug. debug_level=%d.", debug_level);

    return fw_send_msg(session, &msg);
}

static int fw_set_option(struct mvx_session *session,
             struct mvx_fw_set_option *option)
{
    struct mvx_fw_msg msg = {
        .code       = MVX_FW_CODE_SET_OPTION,
        .set_option = *option
    };

    MVX_SESSION_INFO(session, "Firmware req: Set option. code=%d.",
             option->code);

    return fw_send_msg(session, &msg);
}

static bool is_encoder(struct mvx_session *session)
{
    return get_bitstream_port(session) == MVX_DIR_OUTPUT;
}

static int fw_eos(struct mvx_session *session)
{
    struct mvx_fw_msg msg = {
        .code         = MVX_FW_CODE_EOS,
        .eos_is_frame = is_encoder(session) ? true : false
    };
    int ret;

    MVX_SESSION_INFO(session, "Firmware req: Buffer EOS.");

    ret = fw_send_msg(session, &msg);
    if (ret != 0)
        return ret;

    session->port[MVX_DIR_INPUT].flushed = false;

    return 0;
}

static int fw_set_epr_qp(struct mvx_session *session,
             int code,
             struct mvx_buffer_param_qp qp)
{
    struct mvx_fw_set_option option;
    int ret;

    if (qp.qp < 0)
        return -EINVAL;

    if (qp.qp == 0)
        return 0;

    option.code = code;
    option.epr_qp = qp;
    ret = fw_set_option(session, &option);
    if (ret != 0) {
        MVX_SESSION_WARN(session,
                 "Failed to set EPR QP. code=%d, ret=%d.",
                 code, ret);
        return ret;
    }

    return 0;
}
static int fw_set_qp(struct mvx_session *session,
             int code,
             int qp)
{
    struct mvx_fw_set_option option;
    int ret;

    if (qp < 0)
        return -EINVAL;

    if (qp == 0)
        return 0;

    option.code = code;
    option.qp = qp;
    ret = fw_set_option(session, &option);
    if (ret != 0) {
        MVX_SESSION_WARN(session,
                 "Failed to set QP. code=%d, ret=%d.",
                 code, ret);
        return ret;
    }

    return 0;
}

static int fw_set_osd_config(struct mvx_session *session,
             int code,
              struct mvx_osd_config *osd)
{
    struct mvx_fw_set_option option;
    int ret;

    option.code = code;
    option.osd_config = *osd;
    ret = fw_set_option(session, &option);
    if (ret != 0) {
        MVX_SESSION_WARN(session,
                 "Failed to set OSD config. code=%d, ret=%d.",
                 code, ret);
        return ret;
    }

    return 0;
}

static int fw_set_roi_regions(struct mvx_session *session,
             int code,
              struct mvx_roi_config *roi)
{
    struct mvx_fw_set_option option;
    int ret;

    if (roi->num_roi < 0)
        return -EINVAL;

    if (roi->num_roi == 0)
        return 0;

    option.code = code;
    option.roi_config = *roi;
    ret = fw_set_option(session, &option);
    if (ret != 0) {
        MVX_SESSION_WARN(session,
                 "Failed to set ROI. code=%d, ret=%d.",
                 code, ret);
        return ret;
    }

    return 0;
}

static int fw_set_chr_cfg(struct mvx_session *session,
             int code,
              struct mvx_chr_cfg *chr)
{
    struct mvx_fw_set_option option;
    int ret;

    if (chr->num_chr < 0)
        return -EINVAL;

    if (chr->num_chr == 0)
        return 0;

    option.code = code;
    option.chr_cfg = *chr;
    ret = fw_set_option(session, &option);
    if (ret != 0) {
        MVX_SESSION_WARN(session,
                 "Failed to set CHR CFG. code=%d, ret=%d.",
                 code, ret);
        return ret;
    }

    return 0;
}

static int fw_set_enc_stats(struct mvx_session *session,
             int code,
              struct mvx_enc_stats *stats)
{
    struct mvx_fw_set_option option;
    int ret;

    if (stats->flags == 0)
        return 0;

    option.code = code;
    option.enc_stats = *stats;
    ret = fw_set_option(session, &option);
    if (ret != 0) {
        MVX_SESSION_WARN(session,
                 "Failed to set enc stats param. code=%d, ret=%d.",
                 code, ret);
        return ret;
    }

    return 0;
}

static int fw_common_setup(struct mvx_session *session)
{
    int ret = 0;
    struct mvx_fw_set_option option;

    if (session->nalu_format != MVX_NALU_FORMAT_UNDEFINED &&
        session->port[MVX_DIR_INPUT].format != MVX_FORMAT_AV1) {
        option.code = MVX_FW_SET_NALU_FORMAT;
        option.nalu_format = session->nalu_format;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set NALU format.");
            return ret;
        }
    }

    if (session->stream_escaping != MVX_TRI_UNSET) {
        option.code = MVX_FW_SET_STREAM_ESCAPING;
        option.stream_escaping = session->stream_escaping;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set stream escaping.");
            return ret;
        }
    }

    if (session->profiling != 0) {
        option.code = MVX_FW_SET_PROFILING;
        option.profiling = session->profiling;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to enable FW profiling.");
            return ret;
        }
    }
    return ret;
}

/* JPEG standard, Annex K */
static const uint8_t qtbl_chroma_ref[MVX_FW_QUANT_LEN] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

static const uint8_t qtbl_luma_ref[MVX_FW_QUANT_LEN] = {
    16, 11, 10, 16, 24,  40,  51,  61,
    12, 12, 14, 19, 26,  58,  60,  55,
    14, 13, 16, 24, 40,  57,  69,  56,
    14, 17, 22, 29, 51,  87,  80,  62,
    18, 22, 37, 56, 68,  109, 103, 77,
    24, 35, 55, 64, 81,  104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};

void generate_quant_tbl(int quality,
            const uint8_t qtbl_ref[MVX_FW_QUANT_LEN],
            uint8_t qtbl[MVX_FW_QUANT_LEN])
{
    int i;
    int q;

    q = (quality < 50) ? (5000 / quality) : (200 - 2 * quality);

    for (i = 0; i < MVX_FW_QUANT_LEN; ++i) {
        qtbl[i] = ((qtbl_ref[i] * q) + 50) / 100;
        qtbl[i] = min_t(int, qtbl[i], 255);
        qtbl[i] = max_t(int, qtbl[i], 1);
    }
}

static int generate_standards_yuv2rgb_coef(enum mvx_yuv_to_rgb_mode mode,struct mvx_color_conv_coef *color_conv_coef)
{
    static struct mvx_color_conv_coef color_standards[] =
    {
        { {{4769,  4769,  4769}, {0, -1605,  8263}, {6537, -3330,  0}},    {16,  128,    128}},
        { {{4096,  4096,  4096}, {0, -1410,  7258}, {5743, -2925,  0}},    {0 ,  128,    128}},
        { {{4769,  4769,  4769}, {0,  -873,  8652}, {7343, -2183,  0}},    {16,  128,    128}},
        { {{4096,  4096,  4096}, {0, -767,     7601}, {6450, -1917,  0}},    {0 ,  128,    128}},
        { {{4769,  4769,  4769}, {0, -767,     8773}, {6876, -2664,  0}},    {16,  128,    128}},
        { {{4096,  4096,  4096}, {0, -674,     7706}, {6040, -2340,  0}},    {0 ,  128,    128}},
    };

    if(mode <MVX_YUV_TO_RGB_MODE_BT601_LIMT ||    mode >=MVX_YUV_TO_RGB_MODE_MAX)
    {
        mode =MVX_YUV_TO_RGB_MODE_BT601_LIMT;
        //return -EINVAL;
    }

    memcpy(color_conv_coef,&color_standards[mode],sizeof(struct mvx_color_conv_coef));

    MVX_LOG_PRINT(&mvx_log_if, MVX_LOG_DEBUG,
                            "generate_standards_yuv2rgb_coef.mode indx=%d 3x3=[%d %d %d, %d %d %d,%d %d %d],offset=[%d %d %d]",mode,
                               color_conv_coef->coef[0][0],
                               color_conv_coef->coef[0][1],
                               color_conv_coef->coef[0][2],
                               color_conv_coef->coef[1][0],
                               color_conv_coef->coef[1][1],
                               color_conv_coef->coef[1][2],
                               color_conv_coef->coef[2][0],
                               color_conv_coef->coef[2][1],
                               color_conv_coef->coef[2][2],
                               color_conv_coef->offset[0],
                               color_conv_coef->offset[1],
                               color_conv_coef->offset[2]);

    return 0;
}

static int generate_standards_rgb2yuv_coef(enum mvx_rgb_to_yuv_mode mode,struct mvx_rgb2yuv_color_conv_coef *color_conv_coef)
{
    static struct mvx_rgb2yuv_color_conv_coef color_standards[] =
    {
        {{1052, 2065, 401, -607, -1192, 1799, 1799, -1506, -293}, {16,235},{16,240}, {0, 255}},
        {{1225, 2404, 467, -691, -1357, 2048, 2048, -1715, -333}, { 0,255},{ 0,255}, {0, 255}},
        {{ 748, 2516, 254, -412, -1387, 1799, 1799, -1634, -165}, {16,235},{16,240}, {0, 255}},
        {{ 871, 2929, 296, -469, -1579, 2048, 2048, -1860, -188}, { 0,255},{ 0,255}, {0, 255}},
        {{ 924, 2385, 209, -502, -1297, 1799, 1799, -1654, -145}, {16,235},{16,240}, {0, 255}},
        {{1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165}, { 0,255},{ 0,255}, {0, 255}},
    };

    if(mode <MVX_RGB_TO_YUV_MODE_BT601_STUDIO || mode >=MVX_RGB_TO_YUV_MODE_MAX)
    {
        mode =MVX_RGB_TO_YUV_MODE_BT601_STUDIO;
    }

    memcpy(color_conv_coef,&color_standards[mode],sizeof(struct mvx_rgb2yuv_color_conv_coef));

    return 0;
}

static int fw_encoder_setup(struct mvx_session *session)
{
    int ret;
    enum mvx_format codec;
    struct mvx_fw_set_option option;
    enum mvx_direction dir;

    dir = get_bitstream_port(session);
    codec = session->port[dir].format;

#if 0
    pr_info("%s>encoder settings:\n", __func__);
    pr_info("codec %d\n", codec);
    pr_info("frame_rate 0x%lx\n", session->frame_rate);
    pr_info("target_bitrate %d\n", session->target_bitrate);
    pr_info("maximum_bitrate %d\n", session->maximum_bitrate);
    pr_info("rc_enabled %d\n", session->rc_enabled);
    pr_info("rc_type %d\n", session->rc_type);
    pr_info("profile %d\n", session->profile[codec]);
    pr_info("level %d\n", session->level[codec]);
    pr_info("nalu_format %d\n", session->nalu_format);
    pr_info("stream_escaping %d\n", session->stream_escaping);
    pr_info("ignore_stream_headers %d\n", session->ignore_stream_headers);
    pr_info("frame_reordering %d\n", session->frame_reordering);
    pr_info("intbuf_size %ld\n", session->intbuf_size);
    pr_info("p_frames %d\n", session->p_frames);
    pr_info("b_frames %d\n", session->b_frames);
    pr_info("gop_type %d\n", session->gop_type);
    pr_info("cyclic_intra_refresh_mb %d\n", session->cyclic_intra_refresh_mb);
    pr_info("constr_ipred %d\n", session->constr_ipred);
    pr_info("entropy_sync %d\n", session->entropy_sync);
    pr_info("temporal_mvp %d\n", session->temporal_mvp);
    pr_info("tile_rows %d\n", session->tile_rows);
    pr_info("tile_cols %d\n", session->tile_cols);
    pr_info("min_luma_cb_size %d\n", session->min_luma_cb_size);
    pr_info("mb_mask %d\n", session->mb_mask);
    pr_info("entropy_mode %d\n", session->entropy_mode);
    pr_info("multi_slice_mode %d\n", session->multi_slice_mode);
    pr_info("multi_slice_max_mb %d\n", session->multi_slice_max_mb);
    pr_info("vp9_prob_update %d\n", session->vp9_prob_update);
    pr_info("mv_h_search_range %d\n", session->mv_h_search_range);
    pr_info("mv_v_search_range %d\n", session->mv_v_search_range);
    pr_info("bitdepth_chroma %d\n", session->bitdepth_chroma);
    pr_info("bitdepth_luma %d\n", session->bitdepth_luma);
    pr_info("force_chroma_format %d\n", session->force_chroma_format);
    pr_info("rgb_to_yuv %d\n", session->rgb_to_yuv);
    pr_info("band_limit %d\n", session->band_limit);
    pr_info("cabac_init_idc %d\n", session->cabac_init_idc);
    pr_info("qp %d, %d, %d, %d, %d\n", session->qp[codec].i_frame, session->qp[codec].p_frame, session->qp[codec].b_frame, session->qp[codec].min, session->qp[codec].max);
    pr_info("resync_interval %d\n", session->resync_interval);
    pr_info("jpeg_quality %d\n", session->jpeg_quality);
    pr_info("color_desc %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
              session->color_desc.flags, session->color_desc.range, session->color_desc.primaries, session->color_desc.transfer, session->color_desc.matrix,
              session->color_desc.video_format, session->color_desc.aspect_ratio_idc, session->color_desc.sar_width, session->color_desc.sar_height,
              session->color_desc.num_units_in_tick, session->color_desc.time_scale);
    pr_info("crop_left %d\n", session->crop_left);
    pr_info("crop_right %d\n", session->crop_right);
    pr_info("crop_top %d\n", session->crop_top);
    pr_info("crop_bottom %d\n", session->crop_bottom);
    pr_info("sei_userdata %d\n", session->sei_userdata.flags);
    pr_info("nHRDBufsize %d\n", session->nHRDBufsize);
    pr_info("dsl_frame %d, %d\n", session->dsl_frame.width, session->dsl_frame.height);
    pr_info("dsl_ratio %d, %d\n", session->dsl_ratio.hor, session->dsl_ratio.ver);
    pr_info("mvx_ltr %d, %d\n", session->mvx_ltr.mode, session->mvx_ltr.period);
    pr_info("dsl_pos_mode %d\n", session->dsl_pos_mode);
    pr_info("mini_frame_height %d\n", session->mini_frame_height);
    pr_info("init_qpi %d\n", session->init_qpi);
    pr_info("init_qpp %d\n", session->init_qpp);
    pr_info("sao_luma %d\n", session->sao_luma);
    pr_info("sao_chroma %d\n", session->sao_chroma);
    pr_info("qp_delta_i_p %d\n", session->qp_delta_i_p);
    pr_info("ref_rb_en %d\n", session->ref_rb_en);
    pr_info("qpmap_qp_clip_top %d\n", session->qpmap_qp_clip_top);
    pr_info("qpmap_qp_clip_bot %d\n", session->qpmap_qp_clip_bot);
    pr_info("rc_qp_clip_top %d\n", session->rc_qp_clip_top);
    pr_info("rc_qp_clip_bot %d\n", session->rc_qp_clip_bot);
    pr_info("max_qp_i %d\n", session->max_qp_i);
    pr_info("min_qp_i %d\n", session->min_qp_i);
    pr_info("profiling %d\n", session->profiling);
    pr_info("visible_width %d\n", session->visible_width);
    pr_info("visible_height %d\n", session->visible_height);
    pr_info("rc_bit_i_mode %d\n", session->rc_bit_i_mode);
    pr_info("rc_bit_i_ratio %d\n", session->rc_bit_i_ratio);
    pr_info("inter_med_buf_size %d\n", session->inter_med_buf_size);
    pr_info("svct3_level1_period %d\n", session->svct3_level1_period);
    pr_info("reset_gop_pframes %d\n", session->reset_gop_pframes);
    pr_info("reset_ltr_period %d\n", session->reset_ltr_period);
    pr_info("fixedqp %d\n", session->fixedqp);
    pr_info("gdr_number %d\n", session->gdr_number);
    pr_info("gdr_period %d\n", session->gdr_period);
    pr_info("multi_sps_pps %d\n", session->multi_sps_pps);
    pr_info("enable_visual %d\n", session->enable_visual);
    pr_info("scd_enable %d\n", session->scd_enable);
    pr_info("scd_percent %d\n", session->scd_percent);
    pr_info("scd_threshold %d\n", session->scd_threshold);
    pr_info("aq_ssim_en %d\n", session->aq_ssim_en);
    pr_info("aq_neg_ratio %d\n", session->aq_neg_ratio);
    pr_info("aq_pos_ratio %d\n", session->aq_pos_ratio);
    pr_info("aq_qpdelta_lmt %d\n", session->aq_qpdelta_lmt);
    pr_info("aq_init_frm_avg_svar %d\n", session->aq_init_frm_avg_svar);
    pr_info("adaptive_intra_block %d\n", session->adaptive_intra_block);
    pr_info("seamless_mode %d, w %d, h %d\n", session->seamless_target.seamless_mode, session->seamless_target.target_width, session->seamless_target.target_height);
    pr_info("color_conv_mode %d\n", session->color_conv_mode);
    pr_info("use_cust_rgb_to_yuv_mode %d\n", session->use_cust_rgb_to_yuv_mode);
    pr_info("use_cust_color_conv_coef %d\n", session->use_cust_color_conv_coef);
    pr_info("forced_uv_value %d\n", session->forced_uv_value);
    pr_info("dsl_interp_mode %d\n", session->dsl_interp_mode);
    pr_info("disabled_features %d\n", session->disabled_features);
    pr_info("change_pos %d\n", session->change_pos);
    pr_info("enc_src_crop.crop_en %d\n", session->enc_src_crop.crop_en);
#endif

    if (session->profile[codec] != MVX_PROFILE_NONE) {
        option.code = MVX_FW_SET_PROFILE_LEVEL;
        option.profile_level.profile = session->profile[codec];
        option.profile_level.level = session->level[codec];
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set profile/level.");
            return ret;
        }
    }

    if (codec != MVX_FORMAT_JPEG) {
        if (session->rc_type) {
            option.code = MVX_FW_SET_RATE_CONTROL;
            option.rate_control.target_bitrate =
                    session->rc_type ? session->target_bitrate:0;
            option.rate_control.rate_control_mode = session->rc_type;
            if (session->rc_type == MVX_OPT_RATE_CONTROL_MODE_C_VARIABLE) {
                option.rate_control.maximum_bitrate = session->maximum_bitrate;
            }
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to put target bitrate.");
                return ret;
            }
        }

        option.code = MVX_FW_SET_FRAME_RATE;
        option.frame_rate = session->frame_rate;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to put frame rate.");
            return ret;
        }

        if (session->rc_bit_i_mode != 0) {
            option.code = MVX_FW_SET_RC_BIT_I_MODE;
            option.rc_bit_i_mode = session->rc_bit_i_mode;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to put rc bit i mode.");
                return ret;
            }
        }
        if (session->rc_bit_i_ratio != 0) {
            option.code = MVX_FW_SET_RC_BIT_I_RATIO;
            option.rc_bit_i_ratio = session->rc_bit_i_ratio;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to put rc bit i ratio.");
                return ret;
            }
        }

        if (session->multi_sps_pps != 0) {
            option.code = MVX_FW_SET_MULTI_SPS_PPS;
            option.multi_sps_pps = session->multi_sps_pps;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to support multi SPS PSS.");
                return ret;
            }
        }

        if (session->scd_enable != 0) {
            option.code = MVX_FW_SET_SCD_ENABLE;
            option.scd_enable = session->scd_enable;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to enable SCD.");
                return ret;
            }
        }

        if (session->scd_enable != 0 && session->scd_percent >= 0 && session->scd_percent <= 10) {
            option.code = MVX_FW_SET_SCD_PERCENT;
            option.scd_percent = session->scd_percent;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to set SCD percent.");
                return ret;
            }
        }

        if (session->scd_enable != 0 && session->scd_threshold >=0 && session->scd_threshold <= 2047) {
            option.code = MVX_FW_SET_SCD_THRESHOLD;
            option.scd_threshold = session->scd_threshold;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to set SCD threshold.");
                return ret;
            }
        }

        if (session->aq_ssim_en != 0 &&
            (codec == MVX_FORMAT_H264 ||
             codec == MVX_FORMAT_HEVC)) {
            option.code = MVX_FW_SET_AQ_SSIM_EN;
            option.aq_ssim_en = session->aq_ssim_en;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to enable SSIM.");
                return ret;
            }
        }

        if (session->aq_ssim_en != 0 && session->aq_neg_ratio >= 0 &&
            session->aq_neg_ratio <= 63 &&
            (codec == MVX_FORMAT_H264 ||
            codec == MVX_FORMAT_HEVC)) {
            option.code = MVX_FW_SET_AQ_NEG_RATIO;
            option.aq_neg_ratio = session->aq_neg_ratio;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to set AQ negative ratio.");
                return ret;
            }
        }

        if (session->aq_ssim_en != 0 && session->aq_pos_ratio >= 0 &&
            session->aq_pos_ratio <= 63 &&
            (codec == MVX_FORMAT_H264 ||
            codec == MVX_FORMAT_HEVC)) {
            option.code = MVX_FW_SET_AQ_POS_RATIO;
            option.aq_pos_ratio = session->aq_pos_ratio;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to set AQ positive ratio.");
                return ret;
            }
        }

        if (session->aq_ssim_en != 0 && session->aq_qpdelta_lmt >= 0 &&
            session->aq_qpdelta_lmt <= 7 &&
            (codec == MVX_FORMAT_H264 ||
            codec == MVX_FORMAT_HEVC)) {
            option.code = MVX_FW_SET_AQ_QPDELTA_LMT;
            option.aq_qpdelta_lmt = session->aq_qpdelta_lmt;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to set AQ QPDelta LMT.");
                return ret;
            }
        }

        if (session->aq_ssim_en != 0 && session->aq_init_frm_avg_svar >= 0 &&
            session->aq_init_frm_avg_svar <=15 &&
            (codec == MVX_FORMAT_H264 ||
            codec == MVX_FORMAT_HEVC)) {
            option.code = MVX_FW_SET_AQ_INIT_FRM_AVG_SVAR;
            option.aq_init_frm_avg_svar = session->aq_init_frm_avg_svar;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to initial frame variance.");
                return ret;
            }
        }

        if (session->enable_visual != 0) {
            option.code = MVX_FW_SET_VISUAL_ENABLE;
            option.enable_visual = session->enable_visual;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to enable visual.");
                return ret;
            }
            option.code = MVX_FW_SET_ADPTIVE_QUANTISATION;
            option.adapt_qnt = 3;//set to 3 if enable visual
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to set adaptive quantisation.");
                return ret;
            }
        }

        if (session->adaptive_intra_block != 0) {
            option.code = MVX_FW_SET_VISUAL_ENABLE_ADAPTIVE_INTRA_BLOCK;
            option.adaptive_intra_block = session->adaptive_intra_block;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                    "Failed to enable adaptive intra block.");
                return ret;
            }
        }

        if (session->rc_enabled != false) {
            if (session->qp[codec].min < session->qp[codec].max) {
                option.code = MVX_FW_SET_QP_RANGE;
                option.qp_range.min = session->qp[codec].min;
                option.qp_range.max = session->qp[codec].max;
                ret = fw_set_option(session, &option);
                if (ret != 0) {
                    MVX_SESSION_WARN(session,
                             "Failed to set qp range.");
                    return ret;
                }
            }
        }
        if (session->fixedqp != 0) {
            ret = fw_set_qp(session, MVX_FW_SET_FIXED_QP,
                    session->fixedqp);
            if (ret != 0)
                return ret;
        } else {
            if (session->qp[codec].i_frame != 0) {
                ret = fw_set_qp(session, MVX_FW_SET_QP_I,
                        session->qp[codec].i_frame);
                if (ret != 0)
                    return ret;
            }
            if (session->qp[codec].p_frame != 0) {
                ret = fw_set_qp(session, MVX_FW_SET_QP_P,
                        session->qp[codec].p_frame);
                if (ret != 0)
                    return ret;
            }
            if (session->qp[codec].b_frame != 0) {
                ret = fw_set_qp(session, MVX_FW_SET_QP_B,
                        session->qp[codec].b_frame);
                if (ret != 0)
                    return ret;
            }
        }

        if ((session->min_qp_i <= session->max_qp_i) && (session->max_qp_i != 0)) {
            option.code = MVX_FW_SET_QP_RANGE_I;
            option.qp_range.min = session->min_qp_i;
            option.qp_range.max = session->max_qp_i;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set qp range.");
                return ret;
            }
        }

        {
            option.code = MVX_FW_SET_P_FRAMES;
            option.pb_frames = session->p_frames;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set P frames.");
                return ret;
            }
        }

        if (session->b_frames != 0) {
            option.code = MVX_FW_SET_B_FRAMES;
            option.pb_frames = session->b_frames;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set B frames.");
                return ret;
            }
        }

        if (session->gop_type != MVX_GOP_TYPE_NONE) {
            option.code = MVX_FW_SET_GOP_TYPE;
            option.gop_type = session->gop_type;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set GOP type.");
                return ret;
            }
        }

        if (session->inter_med_buf_size != 0) {
            option.code = MVX_FW_SET_INTER_MED_BUF_SIZE;
            option.inter_med_buf_size = session->inter_med_buf_size;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set inter mediate buffer size.");
                return ret;
            }
        }

        if (session->svct3_level1_period != 0) {
            option.code = MVX_FW_SET_SVCT3_LEVEL1_PERIOD;
            option.svct3_level1_period = session->svct3_level1_period;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set svct3_level1_period.");
                return ret;
            }
        }

        if (session->reset_ltr_period != 0) {
            option.code = MVX_FW_SET_LTR_PERIOD;
            option.reset_ltr_period = session->reset_ltr_period;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set ltr period.");
                return ret;
            }
        }

        if (session->reset_gop_pframes != 0) {
            option.code = MVX_FW_SET_GOP_PFRAMES;
            option.reset_gop_pframes = session->reset_gop_pframes;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set gop pframes.");
                return ret;
            }
        }

        if (session->cyclic_intra_refresh_mb != 0) {
            option.code = MVX_FW_SET_INTRA_MB_REFRESH;
            option.intra_mb_refresh =
                session->cyclic_intra_refresh_mb;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set cyclic intra refresh Mb.");
                return ret;
            }
        }

        if (session->constr_ipred != MVX_TRI_UNSET &&
            (codec == MVX_FORMAT_H264 || codec == MVX_FORMAT_HEVC)) {
            option.code = MVX_FW_SET_CONSTR_IPRED;
            option.constr_ipred = session->constr_ipred;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set constr ipred.");
                return ret;
            }
        }
        if (session->change_pos != 0) {
            option.code = MVX_FW_SET_RATE_CONTROL_CHANGE_POS;
            option.change_pos = session->change_pos;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                     "Failed to set change pos for jpeg rc");
                return ret;
            }
        }
    }

    if (codec == MVX_FORMAT_HEVC) {
        if (session->entropy_sync != MVX_TRI_UNSET) {
            option.code = MVX_FW_SET_ENTROPY_SYNC;
            option.entropy_sync = session->entropy_sync;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set entropy sync.");
                return ret;
            }
        }

        if (session->temporal_mvp != MVX_TRI_UNSET) {
            option.code = MVX_FW_SET_TEMPORAL_MVP;
            option.temporal_mvp = session->temporal_mvp;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set temporal mvp.");
                return ret;
            }
        }
        if (session->min_luma_cb_size != 0) {
            option.code = MVX_FW_SET_MIN_LUMA_CB_SIZE;
            option.min_luma_cb_size = session->min_luma_cb_size;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                        "Failed to set min luma cb size.");
                return ret;
            }
        }
    }

    if ((codec == MVX_FORMAT_HEVC ||
         codec == MVX_FORMAT_VP9) &&
        (session->tile_rows != 0 ||
         session->tile_cols != 0)) {
        option.code = MVX_FW_SET_TILES;
        option.tile.rows = session->tile_rows;
        option.tile.cols = session->tile_cols;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set tile dims.");
            return ret;
        }
    }

    if (session->entropy_mode != MVX_ENTROPY_MODE_NONE &&
        codec == MVX_FORMAT_H264) {
        option.code = MVX_FW_SET_ENTROPY_MODE;
        option.entropy_mode = session->entropy_mode;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set entropy mode.");
            return ret;
        }
    }

    if (codec == MVX_FORMAT_H264 ||
        codec == MVX_FORMAT_HEVC) {
        option.code = MVX_FW_SET_SLICE_SPACING_MB;
        if (session->multi_slice_mode ==
            MVX_MULTI_SLICE_MODE_SINGLE)
            option.slice_spacing_mb = 0;
        else
            option.slice_spacing_mb =
                session->multi_slice_max_mb;

        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set slice spacing.");
            return ret;
        }

        option.code = MVX_FW_SET_CABAC_INIT_IDC;
        option.cabac_init_idc = session->cabac_init_idc;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set CABAC init IDC.");
            return ret;
        }
        if (session->crop_left != 0) {
            option.code = MVX_FW_SET_CROP_LEFT;
            option.crop_left = session->crop_left;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                     "Failed to set crop left");
                return ret;
            }
        }
        if (session->crop_right != 0) {
            option.code = MVX_FW_SET_CROP_RIGHT;
            option.crop_right = session->crop_right;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                     "Failed to set crop right");
                return ret;
            }
        }
        if (session->crop_top != 0) {
            option.code = MVX_FW_SET_CROP_TOP;
            option.crop_top = session->crop_top;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                     "Failed to set crop top");
                return ret;
            }
        }
        if (session->crop_bottom != 0) {
            option.code = MVX_FW_SET_CROP_BOTTOM;
            option.crop_bottom = session->crop_bottom;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                     "Failed to set crop bottom");
                return ret;
            }
        }
        if (session->nHRDBufsize != 0) {
            option.code = MVX_FW_SET_HRD_BUF_SIZE;
            option.nHRDBufsize = session->nHRDBufsize;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                     "Failed to set HRD Buffer Size");
                return ret;
            }
        }
        if (session->color_desc.range != 0 || session->color_desc.matrix != 0 ||
            session->color_desc.primaries != 0 || session->color_desc.transfer != 0 ||
            session->color_desc.sar_height != 0 || session->color_desc.sar_width != 0 ||
            session->color_desc.aspect_ratio_idc != 0) {
            struct mvx_fw_set_option option;

            option.code = MVX_FW_SET_COLOUR_DESC;
            option.colour_desc = session->color_desc;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                     "Failed to set vui colour description");
                return ret;
            }
        }

        if (session->sei_userdata.flags) {
            option.code = MVX_FW_SET_SEI_USERDATA;
            option.userdata = session->sei_userdata;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                     "Failed to set sei userdata");
                return ret;
            }
        }

        if (session->mvx_ltr.mode != 0 || session->mvx_ltr.period != 0){
            option.code = MVX_FW_SET_LONG_TERM_REF;
            option.ltr.mode = session->mvx_ltr.mode;
            option.ltr.period = session->mvx_ltr.period;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                "Failed to set ltr mode/period");
                return ret;
            }
        }

        if (session->gdr_number > 1 && session->gdr_period > 1){
            option.code = MVX_FW_SET_GDR_NUMBER;
            option.gdr_number = session->gdr_number;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                "Failed to set gdr number");
                return ret;
            }
            option.code = MVX_FW_SET_GDR_PERIOD;
            option.gdr_period = session->gdr_period;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                "Failed to set gdr period");
                return ret;
            }
        }

    }

    if (codec == MVX_FORMAT_VP9) {
        MVX_SESSION_WARN(session, "VP9 option!");
        option.code = MVX_FW_SET_VP9_PROB_UPDATE;
        option.vp9_prob_update = session->vp9_prob_update;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set VP9 prob update mode.");
            return ret;
        }
    }

    if (session->mv_h_search_range != 0 &&
        session->mv_v_search_range != 0) {
        option.code = MVX_FW_SET_MV_SEARCH_RANGE;
        option.mv.x = session->mv_h_search_range;
        option.mv.y = session->mv_v_search_range;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set motion vector search range.");
            return ret;
        }
    }

    if (session->bitdepth_chroma != 0 &&
        session->bitdepth_luma != 0) {
        option.code = MVX_FW_SET_BITDEPTH;
        option.bitdepth.chroma = session->bitdepth_chroma;
        option.bitdepth.luma = session->bitdepth_luma;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set bitdepth.");
            return ret;
        }
    }

    if (session->force_chroma_format != 0) {
        option.code = MVX_FW_SET_CHROMA_FORMAT;
        option.chroma_format = session->force_chroma_format;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set chroma format.");
            return ret;
        }
    }

    if( session->use_cust_rgb_to_yuv_mode == MVX_CUST_YUV2RGB_MODE_STANDARD )
    {
        option.code = MVX_FW_SET_RGB_TO_YUV_MODE;
        generate_standards_rgb2yuv_coef(session->rgb_to_yuv,&option.rgb2yuv_params);
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set rgb2yuv color conversion mode.");
            return ret;
        }
    }
    else if(session->use_cust_rgb_to_yuv_mode == MVX_CUST_YUV2RGB_MODE_CUSTOMIZED){
        option.code = MVX_FW_SET_RGB_TO_YUV_MODE;
        memcpy(&option.rgb2yuv_params,&session->rgb2yuv_color_conv_coef,sizeof(struct mvx_rgb2yuv_color_conv_coef));
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set rgb2yuv color conversion mode.");
            return ret;
        }
    }

    if (session->band_limit != 0) {
        option.code = MVX_FW_SET_BAND_LIMIT;
        option.band_limit = session->band_limit;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set bandwidth limit.");
            return ret;
        }
    }

    if (session->init_qpi != 0){
        option.code = MVX_FW_SET_INIT_QP_I;
        option.init_qpi = session->init_qpi;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set init qp for I frame.");
            return ret;
        }
    }
    if (session->init_qpp != 0){
        option.code = MVX_FW_SET_INIT_QP_P;
        option.init_qpp = session->init_qpp;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set init qp for P frame.");
            return ret;
        }
    }
    if (session->sao_luma != 0){
        option.code = MVX_FW_SET_SAO_LUMA;
        option.sao_luma = session->sao_luma;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set sao luma.");
            return ret;
        }
    }
    if (session->sao_chroma != 0){
        option.code = MVX_FW_SET_SAO_CHROMA;
        option.sao_chroma = session->sao_chroma;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set sao chroma.");
            return ret;
        }
    }
    if (session->qp_delta_i_p != 0){
        option.code = MVX_FW_SET_QP_DELTA_I_P;
        option.qp_delta_i_p = session->qp_delta_i_p;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set delta qp for I frame and P frame.");
            return ret;
        }
    }
    if (session->ref_rb_en != 0){
        option.code = MVX_FW_SET_QP_REF_RB_EN;
        option.ref_rb_en = session->ref_rb_en;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set ref_rb_en.");
            return ret;
        }
    }
    if (session->rc_qp_clip_top != 0){
        option.code = MVX_FW_SET_RC_CLIP_TOP;
        option.rc_qp_clip_top = session->rc_qp_clip_top;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set rc_qp_clip_top.");
            return ret;
        }
    }
    if (session->rc_qp_clip_bot != 0){
        option.code = MVX_FW_SET_RC_CLIP_BOT;
        option.rc_qp_clip_bot = session->rc_qp_clip_bot;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set rc_qp_clip_bot.");
            return ret;
        }
    }
    if (session->qpmap_qp_clip_top != 0){
        option.code = MVX_FW_SET_QP_MAP_CLIP_TOP;
        option.qpmap_qp_clip_top = session->qpmap_qp_clip_top;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set qpmap_qp_clip_top.");
            return ret;
        }
    }
    if (session->qpmap_qp_clip_top != 0){
        option.code = MVX_FW_SET_QP_MAP_CLIP_BOT;
        option.qpmap_qp_clip_bot = session->qpmap_qp_clip_bot;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set qpmap_qp_clip_bot.");
            return ret;
        }
    }
    if (codec == MVX_FORMAT_JPEG) {
        if (session->resync_interval >= 0) {
            option.code = MVX_FW_SET_RESYNC_INTERVAL;
            option.resync_interval = session->resync_interval;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set resync interval.");
                return ret;
            }
        }

        if (session->jpeg_quality != 0 || session->jpeg_quality_luma != 0 || session->jpeg_quality_chroma != 0) {
            uint8_t qtbl_chroma[MVX_FW_QUANT_LEN];
            uint8_t qtbl_luma[MVX_FW_QUANT_LEN];
            uint32_t quality_luma = session->jpeg_quality_luma != 0 ? session->jpeg_quality_luma : session->jpeg_quality;
            uint32_t quality_chroma = session->jpeg_quality_chroma != 0 ? session->jpeg_quality_chroma : session->jpeg_quality;
            option.code = MVX_FW_SET_QUANT_TABLE;
            if(quality_luma) {
                generate_quant_tbl(quality_luma,
                           qtbl_luma_ref, qtbl_luma);
                option.quant_tbl.luma = qtbl_luma;
            }
            if (quality_chroma) {
                generate_quant_tbl(quality_chroma,
                           qtbl_chroma_ref, qtbl_chroma);
                option.quant_tbl.chroma = qtbl_chroma;
            }

            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set quantization table.");
                return ret;
            }
        }
        if (session->huff_table.type != 0) {
            option.code = MVX_FW_SET_HUFF_TABLE;
            memcpy(&option.huff_table, &session->huff_table, sizeof(struct mvx_huff_table));
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set huff table.");
                return ret;
            }
        }
    }

    if ((session->port[MVX_DIR_INPUT].format ==MVX_FORMAT_YUV444
    ||session->port[MVX_DIR_INPUT].format ==MVX_FORMAT_YUV444_10
    ||session->port[MVX_DIR_INPUT].format ==MVX_FORMAT_YUV420_I420
    ||session->port[MVX_DIR_INPUT].format ==MVX_FORMAT_YUV420_I420_10
    ||session->port[MVX_DIR_INPUT].format ==MVX_FORMAT_Y
    ||session->port[MVX_DIR_INPUT].format ==MVX_FORMAT_Y_10)
    && session->forced_uv_value >= 0 && session->forced_uv_value < 0x400 ) {
            option.code = MVX_FW_SET_ENC_FORCED_UV_VAL;
            option.forced_uv_value = session->forced_uv_value;
            ret = fw_set_option(session, &option);
            if (ret != 0) {
                MVX_SESSION_WARN(session,
                         "Failed to set forced to uv value.");
                return ret;
            }
    }

    if (session->enc_src_crop.width != 0
        && session->enc_src_crop.height !=0
        && session->enc_src_crop.crop_en !=0) {
        option.code = MVX_FW_SET_ENC_SRC_CROPPING;
        memcpy(&option.enc_src_crop,&session->enc_src_crop,sizeof(struct mvx_crop_cfg));
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set enc src crop.");
            return ret;
        }
    }

    if (session->mini_frame_height >= 64) {
        option.code = MVX_FW_SET_MINI_FRAME_HEIGHT;
        option.mini_frame_height = session->mini_frame_height;
         ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                    "Failed to set mini frame buffer cnt.");
            return ret;
        }
    }

    ret = fw_common_setup(session);

    return ret;
}

static int fw_decoder_setup(struct mvx_session *session)
{
    int ret;
    struct mvx_fw_set_option option;

    enum mvx_format codec;
    enum mvx_direction dir;

    dir = get_bitstream_port(session);
    codec = session->port[dir].format;

    if (codec == MVX_FORMAT_VC1 &&
        session->profile[codec] != MVX_PROFILE_NONE) {
        option.code = MVX_FW_SET_PROFILE_LEVEL;
        option.profile_level.profile = session->profile[codec];
        option.profile_level.level = session->level[codec];
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set profile/level.");
            return ret;
        }
    }
    if (codec == MVX_FORMAT_AV1 ) {
        option.code = MVX_FW_SET_DISABLE_FEATURES;
        option.disabled_features = 0x100;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set  disabled features AFBC_LEGACY_REF for AV1.");
            return ret;
        }
    }

    if (session->ignore_stream_headers != MVX_TRI_UNSET) {
        option.code = MVX_FW_SET_IGNORE_STREAM_HEADERS;
        option.ignore_stream_headers = session->ignore_stream_headers;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set ignore stream headers.");
            return ret;
        }
    }

    if (session->frame_reordering != MVX_TRI_UNSET) {
        option.code = MVX_FW_SET_FRAME_REORDERING;
        option.frame_reordering = session->frame_reordering;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set frame reordering.");
            return ret;
        }
    }

    if (session->intbuf_size != 0) {
        option.code = MVX_FW_SET_INTBUF_SIZE;
        option.intbuf_size = session->intbuf_size;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set internal buffer size.");
            return ret;
        }
    }

    if (session->dsl_frame.width != 0 && session->dsl_frame.height != 0) {
        option.code = MVX_FW_SET_DSL_FRAME;
        option.dsl_frame.width = session->dsl_frame.width;
        option.dsl_frame.height = session->dsl_frame.height;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                    "Failed to set DSL frame width/height.");
            return ret;
        }
    }

    if (session->dsl_pos_mode >= 0 && session->dsl_pos_mode <= 2) {
        option.code = MVX_FW_SET_DSL_MODE;
        option.dsl_pos_mode = session->dsl_pos_mode;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                    "Failed to set DSL mode.");
            return ret;
        }
    }

    if (session->dsl_interp_mode >= 0 && session->dsl_interp_mode <= 1) {
        option.code = MVX_FW_SET_DSL_INTERP_MODE;
        option.dsl_interp_mode = session->dsl_interp_mode;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                    "Failed to set DSL INTERP mode.");
            return ret;
        }
    }

    if( mvx_is_rgb24(session->port[MVX_DIR_OUTPUT].format))
    {
        option.code = MVX_FW_SET_DEC_YUV2RGB_PARAMS;

        if(session->use_cust_color_conv_coef)
        {
            ret =0;
            memcpy(&option.yuv2rbg_csc_coef,&session->color_conv_coef,sizeof(struct mvx_color_conv_coef));
        }
        else
        {
            ret =generate_standards_yuv2rgb_coef(session->color_conv_mode,&option.yuv2rbg_csc_coef);
        }
        if(0==ret)
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set yuv2rgb color conversion mode.");
            return ret;
        }
    }
    if (session->disabled_features != 0) {
        option.code = MVX_FW_SET_DISABLE_FEATURES;
        option.disabled_features = session->disabled_features;
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set disabled features.");
            return ret;
        }
    }

    if (session->dec_dst_crop.crop_en !=0
        && session->dec_dst_crop.width > 0
        && session->dec_dst_crop.height > 0) {
        option.code = MVX_FW_SET_DEC_DST_CROPPING;
        memcpy(&option.dec_dst_crop,&session->dec_dst_crop,sizeof(struct mvx_crop_cfg));
        ret = fw_set_option(session, &option);
        if (ret != 0) {
            MVX_SESSION_WARN(session,
                     "Failed to set dec dst crop.");
            return ret;
        }
    }

    ret = fw_common_setup(session);

    return ret;
}

static int fw_initial_setup(struct mvx_session *session)
{
    int ret;
    enum mvx_direction dir;
    enum mvx_format codec;
    struct mvx_fw_set_option option;

    MVX_SESSION_INFO(session, "Firmware initial setup.");

    fw_set_debug(session, 5);

    option.code = MVX_FW_SET_WATCHDOG_TIMEOUT;
    option.watchdog_timeout = fw_watchdog_timeout;
    ret = fw_set_option(session, &option);
    if (ret != 0)
        return ret;

    dir = get_bitstream_port(session);
    codec = session->port[dir].format;

    ret = fw_job(session, 1);
    if (ret != 0)
        return ret;

    if (is_encoder(session))
        ret = fw_encoder_setup(session);
    else
        ret = fw_decoder_setup(session);

    if (ret != 0) {
        MVX_SESSION_WARN(session,
                 "Failed to perform initial setup.\n");
        return ret;
    }

    ret = fw_state_change(session, MVX_FW_STATE_RUNNING);
    if (ret != 0) {
        MVX_SESSION_WARN(session, "Failed to queue state change.");
        return ret;
    }

    ret = fw_ping(session);
    if (ret != 0) {
        MVX_SESSION_WARN(session, "Failed to put ping message.");
        send_event_error(session, ret);
        return ret;
    }

    return ret;
}

/**
 * map_buffer() - Memory map buffer to MVE address space.
 *
 * Return 0 on success, else error code.
 */
static int map_buffer(struct mvx_session *session,
              enum mvx_direction dir,
              struct mvx_buffer *buf)
{
    mvx_mmu_va begin;
    mvx_mmu_va end;
    enum mvx_fw_region region;
    int ret;

    if (mvx_is_bitstream(session->port[dir].format))
        region = MVX_FW_REGION_PROTECTED;
    else if (mvx_is_frame(session->port[dir].format))
        region = MVX_FW_REGION_FRAMEBUF;
    else
        return -EINVAL;

    ret = session->fw.ops.get_region(region, &begin, &end);
    if (ret != 0)
        return ret;

    ret = mvx_buffer_map(buf, begin, end);
    if (ret != 0)
        return ret;

    return 0;
}
static int queue_osd_config(struct mvx_session *session,
            struct mvx_osd_config *osd_cfg)
{
    int ret = 0;
    ret = fw_set_osd_config(session, MVX_FW_SET_OSD_CONFIG,
                    osd_cfg);
    return ret;
}

static int queue_roi_regions(struct mvx_session *session,
            struct mvx_roi_config *roi_cfg)
{
    int ret = 0;
    if ( roi_cfg->qp_present ) {
        ret = fw_set_qp(session, MVX_FW_SET_QP_REGION,
                    roi_cfg->qp);
    }
    if ( roi_cfg->roi_present ) {
        ret = fw_set_roi_regions(session, MVX_FW_SET_ROI_REGIONS,
                    roi_cfg);
    }
    return ret;
}

static int queue_qp_epr(struct mvx_session *session,
            struct mvx_buffer_param_qp *qp)
{
    int ret = 0;
    ret = fw_set_epr_qp(session, MVX_FW_SET_EPR_QP,
                    *qp);

    return ret;
}

static int queue_chr_cfg(struct mvx_session *session,
            struct mvx_chr_cfg *chr_cfg)
{
    int ret = 0;

    ret = fw_set_chr_cfg(session, MVX_FW_SET_CHR_CFG,
                    chr_cfg);
    return ret;
}

static int queue_enc_stats(struct mvx_session *session,
            struct mvx_enc_stats *stats)
{
    int ret = 0;

    ret = fw_set_enc_stats(session, MVX_FW_SET_STATS_MODE,
                    stats);
    return ret;
}
/**
 * queue_buffer() - Put buffer to firmware queue.
 *
 * Return: 0 on success, else error code.
 */
static int queue_buffer(struct mvx_session *session,
            enum mvx_direction dir,
            struct mvx_buffer *buf)
{
    struct mvx_session_port *port = &session->port[dir];
    struct mvx_fw_msg msg;
    struct mvx_seamless_target *seamless = &session->seamless_target;
    unsigned int width;
    unsigned int height;
    unsigned int stride[MVX_BUFFER_NPLANES];
    unsigned i;
    /*
     * Vb2 cannot allocate buffers with bidirectional mapping, therefore
     * proper direction should be set.
     */
    enum dma_data_direction dma_dir =
        (dir == MVX_DIR_OUTPUT) ? DMA_FROM_DEVICE : DMA_TO_DEVICE;

    int ret;
    memset(stride,0,sizeof(stride));
    if (dir == MVX_DIR_OUTPUT) {
        port->scaling_shift = (buf->flags & MVX_BUFFER_FRAME_FLAG_SCALING_MASK) >> 14;
    }
    if (mvx_buffer_is_mapped(buf) == false) {
        ret = map_buffer(session, dir, buf);
        if (ret != 0)
            return ret;
    }
    if (dir == MVX_DIR_OUTPUT && port->buffer_allocated < port->buffer_min) {
        buf->flags |= MVX_BUFFER_FRAME_NEED_REALLOC;
        return -EAGAIN;
    }
    /*
     * Update frame dimensions. They might have changed due to a resolution
     * change.
     */
     if(MVX_DIR_OUTPUT == dir
        && session->port[MVX_DIR_INPUT].format <= MVX_FORMAT_BITSTREAM_LAST
        && seamless->seamless_mode !=0)
     {
        width = seamless->target_width < port->width ? port->width : seamless->target_width;
        height = seamless->target_height < port->height  ? port->height : seamless->target_height;
        for(i=0;i<MVX_BUFFER_NPLANES;i++)
        {
            stride[i]=seamless->target_stride[i] < port->stride[i] ? port->stride[i] : seamless->target_stride[i];
        }
     }
     else
     {
        width = port->width;
        height = port->height;
        for(i=0;i<MVX_BUFFER_NPLANES;i++)
        {
            stride[i]=port->stride[i];
        }
     }
     if (mvx_is_afbc(port->format) != false) {
        port->afbc_width = DIV_ROUND_UP(width, 16 << (!!(buf->flags & MVX_BUFFER_AFBC_32X8_SUPERBLOCK)));
        ret = mvx_buffer_afbc_set(buf, port->format, width,
                      height, port->afbc_width,
                      port->size[0], port->interlaced);
        if (ret != 0)
            return ret;
    } else if (mvx_is_frame(port->format) != false) {
        ret = mvx_buffer_frame_set(buf, port->format, width,
                       height, stride,
                       port->size,
                       port->interlaced);
        if (ret != 0)
            return ret;

    }

    ret = mvx_buffer_synch(buf, dma_dir);
    if (ret != 0)
        return ret;

    msg.code = MVX_FW_CODE_BUFFER;
    msg.buf = buf;

    MVX_SESSION_INFO(session,
             "Firmware req: Buffer. dir=%u, len=[%u, %u, %u], flags=0x%08x, eos=%u, interlace=%u",
             buf->dir,
             buf->planes[0].filled,
             buf->planes[1].filled,
             buf->planes[2].filled,
             buf->flags,
             (buf->flags & MVX_BUFFER_EOS) != 0,
             (buf->flags & MVX_BUFFER_INTERLACE) != 0);

    ret = session->fw.ops.put_message(&session->fw, &msg);
    if (ret != 0)
        goto send_error;

    port->buffer_count++;
    port->flushed = false;
    if (dir == MVX_DIR_OUTPUT && port->isreallocting == true) {
        port->isreallocting = false;
    }
    ret = send_irq(session);
    if (ret != 0)
        goto send_error;

    return 0;

send_error:
    send_event_error(session, ret);
    return ret;
}

/**
 * queue_pending_buffers() - Queue pending buffers.
 *
 * Buffer that are queued when the port is still stream off will be put in the
 * pending queue. Once both input- and output ports are stream on the pending
 * buffers will be forwarded to the firmware.
 *
 * Return: 0 on success, else error code.
 */
static int queue_pending_buffers(struct mvx_session *session,
                 enum mvx_direction dir)
{
    struct mvx_buffer *buf;
    struct mvx_buffer *tmp;
    int roi_config_num = 0;
    int roi_config_index = 0;
    int qp_num = 0;
    int qp_index = 0;
    int chr_cfg_num = 0;
    int chr_cfg_index = 0;
    int enc_stats_num = 0;
    int enc_stats_index = 0;
    int osd_cfg_index = 0;
    int osd_cfg_num = 0;
    int pending_buf_idx = 0;
    int osd_buffer_idx = 0;
    struct mvx_roi_config roi_config;
    int ret = 0;

    if (dir == MVX_DIR_INPUT && session->port[dir].roi_config_num > 0) {
        roi_config_num = session->port[dir].roi_config_num;
    }
    if (dir == MVX_DIR_INPUT && session->port[dir].qp_num > 0) {
        qp_num = session->port[dir].qp_num;
    }
    if (dir == MVX_DIR_INPUT && session->port[dir].chr_cfg_num > 0) {
        chr_cfg_num = session->port[dir].chr_cfg_num;
    }
    if (dir == MVX_DIR_INPUT && session->port[dir].enc_stats_num > 0) {
        enc_stats_num = session->port[dir].enc_stats_num;
    }
    if (dir == MVX_DIR_INPUT && session->port[dir].osd_cfg_num > 0) {
        osd_cfg_num = session->port[dir].osd_cfg_num;
    }
    list_for_each_entry_safe(buf, tmp, &session->port[dir].buffer_queue,
                    head) {
        if ((buf->flags & MVX_BUFFER_FRAME_FLAG_ROI) == MVX_BUFFER_FRAME_FLAG_ROI &&
            roi_config_index < roi_config_num) {
            roi_config = session->port[dir].roi_config_queue[roi_config_index];
            ret = queue_roi_regions(session, &roi_config);
            roi_config_index++;
        }
        if ((buf->flags & MVX_BUFFER_FRAME_FLAG_GENERAL) == MVX_BUFFER_FRAME_FLAG_GENERAL &&
                            qp_index < qp_num &&  dir == MVX_DIR_INPUT) {
            ret = queue_qp_epr(session, &session->port[dir].qp_queue[qp_index]);
            qp_index++;
        }
        if ((buf->flags & MVX_BUFFER_FRAME_FLAG_CHR) == MVX_BUFFER_FRAME_FLAG_CHR &&
                            chr_cfg_index < chr_cfg_num) {
            ret = queue_chr_cfg(session, &session->port[dir].chr_cfg_queue[chr_cfg_index]);
            chr_cfg_index++;
        }
        if (enc_stats_index < enc_stats_num &&
            session->port[dir].enc_stats_queue[enc_stats_index].pic_index == pending_buf_idx ) {
            ret = queue_enc_stats(session, &session->port[dir].enc_stats_queue[enc_stats_index]);
            enc_stats_index++;
        }
        if (osd_cfg_index < osd_cfg_num &&
            session->port[dir].osd_cfg_queue[osd_cfg_index].pic_index == osd_buffer_idx ) {
            ret = queue_osd_config(session, &session->port[dir].osd_cfg_queue[osd_cfg_index]);
            osd_cfg_index++;
        }
        ret = queue_buffer(session, dir, buf);
        pending_buf_idx++;
        if (!(buf->flags & MVX_BUFFER_FRAME_FLAG_OSD_MASK)) {
            osd_buffer_idx++;//check for yuv bffer
        }
        if ((buf->flags & MVX_BUFFER_FRAME_NEED_REALLOC) == MVX_BUFFER_FRAME_NEED_REALLOC) {
            session->event(session, MVX_SESSION_EVENT_BUFFER, buf);
        } else if (ret != 0) {
            break;
        }
        list_del(&buf->head);
    }

    session->port[dir].roi_config_num = 0;
    session->port[dir].qp_num = 0;
    session->port[dir].chr_cfg_num = 0;
    session->port[dir].enc_stats_num = 0;
    return ret;
}

/**
 * fw_bin_ready() - Complete firmware configuration.
 *
 * The firmware binary load has completed and the firmware configuration can
 * begin.
 *
 * If the session is no longer 'stream on' (someone issued 'stream off' before
 * the firmware load completed) the firmware binary is put back to the cache.
 *
 * Else the the client session is registered and the firmware instance is
 * constructed.
 */
static void fw_bin_ready(struct mvx_fw_bin *bin,
             void *arg,
             bool same_thread)
{
    struct mvx_session *session = arg;
    int lock_failed = 1;
    int ret;

    /*
     * Only lock the mutex if the firmware binary was loaded by a
     * background thread.
     */
    if (same_thread == false) {
        lock_failed = mutex_lock_interruptible(session->isession.mutex);
        if (lock_failed != 0) {
            send_event_error(session, lock_failed);
            goto put_fw_bin;
        }
    }

    /* Return firmware binary if session is no longer 'stream on'. */
    if (!is_stream_on(session))
        goto put_fw_bin;

    /* Create client session. */
    session->isession.ncores = session->client_ops->get_ncores(
        session->client_ops);
    session->isession.l0_pte = mvx_mmu_set_pte(
        MVX_ATTR_PRIVATE, virt_to_phys(session->mmu.page_table),
        MVX_ACCESS_READ_WRITE);

    session->csession = session->client_ops->register_session(
        session->client_ops, &session->isession);
    if (IS_ERR(session->csession)) {
        ret = PTR_ERR(session->csession);
        send_event_error(session, ret);
        goto put_fw_bin;
    }

    /* Construct the firmware instance. */
    ret = mvx_fw_factory(&session->fw, bin, &session->mmu,
                 session, session->client_ops, session->csession,
                 session->isession.ncores,
                 session->dentry);
    if (ret != 0) {
        send_event_error(session, ret);
        goto unregister_csession;
    }

    session->fw_bin = bin;

    mvx_fw_cache_log(bin, session->csession);

    ret = fw_initial_setup(session);
    if (ret != 0)
        goto unregister_csession;

    ret = queue_pending_buffers(session, MVX_DIR_INPUT);
    if (ret != 0)
        goto unregister_csession;

    ret = queue_pending_buffers(session, MVX_DIR_OUTPUT);
    if (ret != 0)
        goto unregister_csession;

    if (lock_failed == 0)
        mutex_unlock(session->isession.mutex);

    mvx_session_put(session);

    return;

unregister_csession:
    session->client_ops->unregister_session(session->csession);
    session->csession = NULL;

put_fw_bin:
    mvx_fw_cache_put(session->cache, bin);
    session->fw_bin = NULL;

    if (lock_failed == 0)
        mutex_unlock(session->isession.mutex);

    mvx_session_put(session);
}

static int calc_afbc_size(struct mvx_session *session,
              enum mvx_format format,
              unsigned int width,
              unsigned int height,
              bool tiled_headers,
              bool tiled_body,
              bool superblock,
              bool interlaced)
{
    static const unsigned int mb_header_size = 16;
    unsigned int payload_align = 128;
    unsigned int mb_size;
    int size;

    /* Calculate width and height in super blocks. */
    if (superblock != false) {
        width = DIV_ROUND_UP(width, 32);
        height = DIV_ROUND_UP(height, 8) + 1;
    } else {
        width = DIV_ROUND_UP(width, 16);
        height = DIV_ROUND_UP(height, 16) + 1;
    }

    /* Round up size to 8x8 tiles. */
    if (tiled_headers != false || tiled_body != false) {
        width = roundup(width, 8);
        height = roundup(height, 8);
    }

    switch (format) {
    case MVX_FORMAT_YUV420_AFBC_8:
        mb_size = 384;
        break;
    case MVX_FORMAT_YUV420_AFBC_10:
        mb_size = 480;
        break;
    case MVX_FORMAT_YUV422_AFBC_8:
        mb_size = 512;
        break;
    case MVX_FORMAT_YUV422_AFBC_10:
        mb_size = 656;
        break;
    default:
        MVX_SESSION_WARN(session,
                 "Unsupported AFBC format. format=%u.",
                 format);
        return -EINVAL;
    }

    /* Round up tiled body to 128 byte boundary. */
    if (tiled_body != false)
        mb_size = roundup(mb_size, payload_align);

    if (interlaced != false)
        height = DIV_ROUND_UP(height, 2);

    /* Calculate size of AFBC makroblock headers. */
    size = roundup(width * height * mb_header_size, payload_align);
    size += roundup(width * height * mb_size, payload_align);

    if (interlaced != false)
        size *= 2;

    return size;
}

static int try_format(struct mvx_session *session,
              enum mvx_direction dir,
              enum mvx_format format,
              unsigned int *width,
              unsigned int *height,
              uint8_t *nplanes,
              unsigned int *stride,
              unsigned int *size,
              bool *interlaced)
{
    int ret = 0;

    /* Limit width and height to 8k. */
    *width = min_t(unsigned int, *width, 32768);
    *height = min_t(unsigned int, *height, 32768);

    /* Stream dimensions are dictated by the input port. */
    if (dir == MVX_DIR_OUTPUT) {
        *width = session->port[MVX_DIR_INPUT].width >> session->port[MVX_DIR_OUTPUT].scaling_shift;
        *height = session->port[MVX_DIR_INPUT].height >> session->port[MVX_DIR_OUTPUT].scaling_shift;
    }
    if (session->dsl_frame.width != 0 && session->dsl_frame.height != 0) {
        *width = session->dsl_frame.width;
        *height = session->dsl_frame.height;
    } else if (dir == MVX_DIR_OUTPUT && (session->dsl_ratio.hor != 1 || session->dsl_ratio.ver !=  1)) {
        *width = session->port[MVX_DIR_INPUT].width / session->dsl_ratio.hor;
        *height = session->port[MVX_DIR_INPUT].height / session->dsl_ratio.ver;
        *width &= ~(1);
        *height &= ~(1);
    }
    if(dir == MVX_DIR_OUTPUT &&  mvx_is_frame(format))
    {
        if( session->dec_dst_crop.crop_en !=0
            && session->dec_dst_crop.width >0
            && session->dec_dst_crop.height >0
            && session->dec_dst_crop.width +session->dec_dst_crop.x <= session->port[MVX_DIR_INPUT].width
            && session->dec_dst_crop.height +session->dec_dst_crop.y <= session->port[MVX_DIR_INPUT].height)
        {
            *width=session->dec_dst_crop.width;
            *height=session->dec_dst_crop.height;
        }
    }
    /* Interlaced input is not supported by the firmware. */
    if (dir == MVX_DIR_INPUT)
        *interlaced = false;

    if (mvx_is_afbc(format) != false) {
        unsigned int afbc_alloc_bytes =
            session->port[dir].afbc_alloc_bytes;
        if (*nplanes <= 0)
            size[0] = 0;

        if (dir == MVX_DIR_INPUT) {
            /* it is basically a worst-case calcualtion based on a size rounded up to tile size*/
            int s1 = calc_afbc_size(session, format, *width,
                           *height, true, true, false, //*height, false, false, false,
                           *interlaced);
            int s2 = calc_afbc_size(session, format, *width,
                           *height, true, true, true, //*height, false, false, false,
                           *interlaced);
            int s = max_t(unsigned int, s1, s2);
            if (s < 0)
                return s;

            size[0] = max_t(unsigned int, size[0], s);
        }

        if (*interlaced != false)
            afbc_alloc_bytes *= 2;

        size[0] = max_t(unsigned int, size[0],
                afbc_alloc_bytes);
        size[0] = roundup(size[0], PAGE_SIZE);

        *nplanes = 1;
    } else if (mvx_is_frame(format) != false) {
        uint32_t tmp_height = session->mini_frame_height >= 64? session->mini_frame_height : *height;
        ret = mvx_buffer_frame_dim(format, *width, tmp_height, nplanes,
                       stride, size, session->setting_stride);
    } else {
        /*
         * For compressed formats the size should be the maximum number
         * of bytes an image is expected to become. This is calculated
         * as width * height * 2 B/px / 2. Size should be at least one
         * page.
         */

        stride[0] = 0;

        if (*nplanes <= 0)
            size[0] = 0;

        size[0] = max_t(unsigned int, size[0], PAGE_SIZE);
        size[0] = max_t(unsigned int, size[0], (*width) * (*height));
        size[0] = min_t(unsigned int, size[0], (8192) * (8192));////for large jpeg,bit stream buffer no need to set too large,just clip to 8k*8k
        size[0] = roundup(size[0], PAGE_SIZE);

        *nplanes = 1;
    }

    return ret;
}

static void watchdog_work(struct work_struct *work)
{
    struct mvx_session *session =
        container_of(work, struct mvx_session, watchdog_work);
    int ret;

    mutex_lock(session->isession.mutex);

    MVX_SESSION_WARN(session, "Watchdog timeout. count=%u.",
             session->watchdog_count);

    /* Print debug information. */
    print_debug(session);

    if (session->watchdog_count++ < 2) {
        /* Request firmware to dump its state. */
        fw_dump(session);

        /* Restart watchdog. */
        watchdog_start(session, 3000);
    } else {
        send_event_error(session, -ETIME);
    }

    ret = kref_put(&session->isession.kref, session->isession.release);
    if (ret != 0)
        return;

    mutex_unlock(session->isession.mutex);
}

static void watchdog_timeout(struct timer_list *timer)
{
    struct mvx_session *session =
        container_of(timer, struct mvx_session, watchdog_timer);

    queue_work(system_unbound_wq, &session->watchdog_work);
}

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
static void watchdog_timeout_legacy(unsigned long data)
{
    watchdog_timeout((struct timer_list *)data);
}

#endif

/****************************************************************************
 * Exported functions
 ****************************************************************************/

int mvx_session_construct(struct mvx_session *session,
              struct device *dev,
              struct mvx_client_ops *client_ops,
              struct mvx_fw_cache *cache,
              struct mutex *mutex,
              void (*destructor)(struct mvx_session *session),
              void (*event)(struct mvx_session *session,
                    enum mvx_session_event event,
                    void *arg),
              struct dentry *dentry)
{
    int i;
    int ret;

    if (event == NULL || destructor == NULL)
        return -EINVAL;

    memset(session, 0, sizeof(*session));
    memset(session->setting_stride, 0, sizeof(session->setting_stride));
    memset(session->port[MVX_DIR_INPUT].stride, 0, sizeof(session->port[MVX_DIR_INPUT].stride));
    memset(session->port[MVX_DIR_INPUT].size, 0, sizeof(session->port[MVX_DIR_INPUT].size));
    memset(session->port[MVX_DIR_OUTPUT].stride, 0, sizeof(session->port[MVX_DIR_OUTPUT].stride));
    memset(session->port[MVX_DIR_OUTPUT].size, 0, sizeof(session->port[MVX_DIR_OUTPUT].size));
    memset(session->port[MVX_DIR_INPUT].display_size, 0, sizeof(session->port[MVX_DIR_INPUT].display_size));
    memset(session->port[MVX_DIR_OUTPUT].display_size, 0, sizeof(session->port[MVX_DIR_OUTPUT].display_size));
    session->dev = dev;
    session->client_ops = client_ops;
    session->cache = cache;
    kref_init(&session->isession.kref);
    session->isession.release = session_destructor;
    session->isession.mutex = mutex;
    session->destructor = destructor;
    session->event = event;
    session->fw_event.fw_bin_ready = fw_bin_ready;
    session->fw_event.arg = session;
    session->fw_state = MVX_FW_STATE_STOPPED;
    init_waitqueue_head(&session->waitq);
    session->dentry = dentry;
    session->port[MVX_DIR_INPUT].buffer_min = 1;
    session->port[MVX_DIR_OUTPUT].buffer_min = 1;
    session->port[MVX_DIR_INPUT].buffer_allocated = 0;//1;
    session->port[MVX_DIR_OUTPUT].buffer_allocated = 0;//1;
    session->port[MVX_DIR_INPUT].scaling_shift = 0;
    session->port[MVX_DIR_OUTPUT].scaling_shift = 0;
    session->stream_escaping = MVX_TRI_UNSET;
    session->ignore_stream_headers = MVX_TRI_UNSET;
    session->frame_reordering = MVX_TRI_UNSET;
    session->constr_ipred = MVX_TRI_UNSET;
    session->entropy_sync = MVX_TRI_UNSET;
    session->temporal_mvp = MVX_TRI_UNSET;
    session->resync_interval = -1;
    session->port[MVX_DIR_OUTPUT].roi_config_num = 0;
    session->port[MVX_DIR_INPUT].roi_config_num = 0;
    session->port[MVX_DIR_OUTPUT].qp_num = 0;
    session->port[MVX_DIR_INPUT].qp_num = 0;
    session->crop_left = 0;
    session->crop_right = 0;
    session->crop_top = 0;
    session->crop_bottom = 0;
    session->dsl_ratio.hor = 1;
    session->dsl_ratio.ver = 1;
    session->dsl_pos_mode = -1;//disable by default
    session->profiling = 0;
    session->rc_bit_i_mode = 0;
    session->rc_bit_i_ratio = 0;
    session->multi_sps_pps = 0;
    session->enable_visual = 0;
    session->forced_uv_value =0x400;
    session->dsl_interp_mode =0xffff;
    session->color_conv_mode = MVX_YUV_TO_RGB_MODE_BT601_LIMT;
    session->use_cust_color_conv_coef =false;
    session->use_cust_rgb_to_yuv_mode = MVX_CUST_YUV2RGB_MODE_UNSET;
    memset(&session->color_conv_coef,0,sizeof(struct mvx_color_conv_coef));
    memset(&session->enc_src_crop,0,sizeof(struct mvx_crop_cfg));
    memset(&session->dec_dst_crop,0,sizeof(struct mvx_crop_cfg));
    memset(&session->seamless_target,0,sizeof(struct mvx_seamless_target));
    memset(&session->osd_info, 0, sizeof(session->osd_info));
    memset(&session->huff_table, 0, sizeof(session->huff_table));
    memset(&session->sei_userdata, 0, sizeof(session->sei_userdata));
    memset(&session->color_desc, 0, sizeof(session->color_desc));

    ret = mvx_mmu_construct(&session->mmu, session->dev);
    if (ret != 0)
        return ret;

    for (i = 0; i < MVX_DIR_MAX; i++)
        INIT_LIST_HEAD(&session->port[i].buffer_queue);

#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
    timer_setup(&session->watchdog_timer, watchdog_timeout, 0);
#else
    setup_timer(&session->watchdog_timer, watchdog_timeout_legacy,
            (uintptr_t)&session->watchdog_timer);
#endif
    INIT_WORK(&session->watchdog_work, watchdog_work);

    return 0;
}

void mvx_session_destruct(struct mvx_session *session)
{
    /* Destruct the session object. */

    MVX_SESSION_INFO(session, "Destroy session.");

    release_fw_bin(session);
    mvx_mmu_destruct(&session->mmu);
}

void mvx_session_get(struct mvx_session *session)
{
    kref_get(&session->isession.kref);
}

int mvx_session_put(struct mvx_session *session)
{
    return kref_put(&session->isession.kref,
            session->isession.release);
}

void mvx_session_get_formats(struct mvx_session *session,
                 enum mvx_direction dir,
                 uint64_t *formats)
{
    uint64_t fw_formats;

    session->client_ops->get_formats(session->client_ops, dir, formats);
    mvx_fw_cache_get_formats(session->cache, dir, &fw_formats);

    *formats &= fw_formats;
}

int mvx_session_try_format(struct mvx_session *session,
               enum mvx_direction dir,
               enum mvx_format format,
               unsigned int *width,
               unsigned int *height,
               uint8_t *nplanes,
               unsigned int *stride,
               unsigned int *size,
               bool *interlaced)
{
    return try_format(session, dir, format, width, height, nplanes,
              stride, size, interlaced);
}

int mvx_session_set_format(struct mvx_session *session,
               enum mvx_direction dir,
               enum mvx_format format,
               unsigned int *width,
               unsigned int *height,
               uint8_t *nplanes,
               unsigned int *stride,
               unsigned int *size,
               bool *interlaced)
{
    struct mvx_session_port *port = &session->port[dir];
    int ret;

    if (session->error != 0)
        return session->error;

    if (is_stream_on(session) != false)
        return -EBUSY;

    ret = try_format(session, dir, format, width, height, nplanes,
             stride, size, interlaced);
    if (ret != 0)
        return ret;

    /*
     * If the bitstream format changes, then the firmware binary must be
     * released.
     */
    if (mvx_is_bitstream(port->format) != false &&
        format != port->format) {
        if (IS_ERR(session->fw_bin) != false) {
            MVX_SESSION_WARN(session,
                     "Can't set format when firmware binary is pending. dir=%d.",
                     dir);
            return -EINVAL;
        }

        release_fw_bin(session);
    }

    /* Update port settings. */
    port->format = format;
    port->width = *width;
    port->height = *height;
    port->nplanes = *nplanes;
    port->interlaced = *interlaced;
    memcpy(port->stride, stride, sizeof(*stride) * MVX_BUFFER_NPLANES);
    memcpy(port->size, size, sizeof(*size) * MVX_BUFFER_NPLANES);

    /* TODO AFBC width will have to be provided by user space. */
    if (dir == MVX_DIR_INPUT)
        port->afbc_width = DIV_ROUND_UP(*width, 16);

    /* Input dimensions dictate output dimensions. */
    if (dir == MVX_DIR_INPUT) {
        struct mvx_session_port *p = &session->port[MVX_DIR_OUTPUT];
        (void)try_format(session, MVX_DIR_OUTPUT, p->format, &p->width,
                 &p->height, &p->nplanes, p->stride, p->size,
                 &p->interlaced);
    }

    return 0;
}

int mvx_session_qbuf(struct mvx_session *session,
             enum mvx_direction dir,
             struct mvx_buffer *buf)
{
    int ret;
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) == false ||
        session->port[dir].is_flushing != false) {
        list_add_tail(&buf->head, &session->port[dir].buffer_queue);
        return 0;
    }

    ret = queue_buffer(session, dir, buf);
    if (ret != 0)
        return ret;

    ret = switch_in(session);
    if (ret != 0)
        return ret;

    return 0;
}

int mvx_session_send_eos(struct mvx_session *session)
{
    struct mvx_session_port *port = &session->port[MVX_DIR_OUTPUT];
    struct mvx_buffer *buf;

    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return fw_eos(session);

    if (list_empty(&port->buffer_queue) != false) {
        MVX_SESSION_WARN(session,
                 "Unable to signal EOS. Output buffer queue empty.");
        return 0;
    }

    buf = list_first_entry(&port->buffer_queue, struct mvx_buffer, head);
    list_del(&buf->head);

    mvx_buffer_clear(buf);
    buf->flags |= MVX_BUFFER_EOS;

    session->event(session, MVX_SESSION_EVENT_BUFFER, buf);

    return 0;
}

int mvx_session_streamon(struct mvx_session *session,
             enum mvx_direction dir)
{
    enum mvx_direction bdir;
    struct mvx_hw_ver hw_ver;
    enum mvx_direction i;
    int ret;

    MVX_SESSION_INFO(session, "Stream on. dir=%u.", dir);

    /* Verify that we don't enable an already activated port. */
    if (session->port[dir].stream_on != false)
        return 0;

    session->port[dir].stream_on = true;

    /* Check that both ports are stream on. */
    if (!is_stream_on(session))
        return 0;

    /* Verify that a firmware binary load is not in progress. */
    if (IS_ERR(session->fw_bin)) {
        ret = PTR_ERR(session->fw_bin);
        goto disable_port;
    }

    /* If a firmware binary is already loaded, then we are done. */
    if (session->fw_bin != NULL) {
        ret = wait_pending(session);
        if (ret != 0)
            goto disable_port;

        ret = fw_state_change(session, MVX_FW_STATE_RUNNING);
        if (ret != 0)
            goto disable_port;

        return 0;
    }

    bdir = get_bitstream_port(session);
    if (bdir >= MVX_DIR_MAX) {
        MVX_SESSION_WARN(session,
                 "Session only support decoding and encoding, but not transcoding. input_format=%u, output_format=%u.",
                 session->port[MVX_DIR_INPUT].format,
                 session->port[MVX_DIR_OUTPUT].format);
        ret = -EINVAL;
        goto disable_port;
    }

    /* Verify that client can handle input and output formats. */
    for (i = MVX_DIR_INPUT; i < MVX_DIR_MAX; i++) {
        uint64_t formats;

        session->client_ops->get_formats(session->client_ops,
                         i, &formats);

        if (!mvx_test_bit(session->port[i].format, &formats)) {
            MVX_SESSION_WARN(session,
                     "Client cannot support requested formats. input_format=%u, output_format=%u.",
                     session->port[MVX_DIR_INPUT].format,
                     session->port[MVX_DIR_OUTPUT].format);
            ret = -ENODEV;
            goto disable_port;
        }
    }

    /* Increment session reference count and flag fw bin as pending. */
    mvx_session_get(session);
    session->fw_bin = ERR_PTR(-EINPROGRESS);
    session->client_ops->get_hw_ver(session->client_ops, &hw_ver);

    /* Requesting firmware binary to be loaded. */
    ret = mvx_fw_cache_get(session->cache, session->port[bdir].format,
                   bdir, &session->fw_event, &hw_ver,
                   session->isession.securevideo);
    if (ret != 0) {
        session->port[dir].stream_on = false;
        session->fw_bin = NULL;
        mvx_session_put(session);
        return ret;
    }

    return 0;

disable_port:
    session->port[dir].stream_on = false;

    return ret;
}

int mvx_session_streamoff(struct mvx_session *session,
              enum mvx_direction dir)
{
    struct mvx_session_port *port = &session->port[dir];
    struct mvx_buffer *buf;
    struct mvx_buffer *tmp;
    int ret = 0;

    MVX_SESSION_INFO(session, "Stream off. dir=%u.", dir);

    port->stream_on = false;

    if (is_fw_loaded(session) != false) {
        /*
         * Flush the ports if at least one buffer has been queued
         * since last flush.
         */
        if (port->flushed == false && port->is_flushing == false) {
            ret = wait_pending(session);
            if (ret != 0)
                goto dequeue_buffers;
            if (!(dir == MVX_DIR_OUTPUT && port->isreallocting == true)) {
                ret = fw_state_change(session, MVX_FW_STATE_STOPPED);
                if (ret != 0)
                    goto dequeue_buffers;

                ret = fw_flush(session, dir);
                if (ret != 0)
                    goto dequeue_buffers;
            }
            ret = wait_pending(session);
            if (ret != 0)
                goto dequeue_buffers;

            send_irq(session);
        }
    }
dequeue_buffers:
    if (ret != 0)
        session_unregister(session);

    /* Return buffers in pending queue. */
    list_for_each_entry_safe(buf, tmp, &port->buffer_queue, head) {
        list_del(&buf->head);
        session->event(session, MVX_SESSION_EVENT_BUFFER, buf);
    }

    return 0;
}

static void handle_fw_message(struct mvx_session *session,
                  struct mvx_fw_msg *msg)
{
    switch (msg->code) {
    case MVX_FW_CODE_ALLOC_PARAM: {
        struct mvx_session_port *input = &session->port[MVX_DIR_INPUT];
        struct mvx_session_port *output =
            &session->port[MVX_DIR_OUTPUT];
        /* Update input port. */
        input->width = msg->alloc_param.width;
        input->height = msg->alloc_param.height;

        try_format(session, MVX_DIR_INPUT, input->format, &input->width,
               &input->height, &input->nplanes, input->stride,
               input->size, &input->interlaced);

        /*
         * Update output port. Set number of valid planes to 0 to force
         * stride to be recalculated.
         */

        output->nplanes = 0;
        output->afbc_alloc_bytes = msg->alloc_param.afbc_alloc_bytes;
        output->afbc_width = msg->alloc_param.afbc_width;
        try_format(session, MVX_DIR_OUTPUT, output->format,
               &output->width, &output->height, &output->nplanes,
               output->stride, output->size,
               &output->interlaced);

        MVX_SESSION_INFO(session,
                 "Firmware rsp: Alloc param. width=%u, height=%u, nplanes=%u, size=[%u, %u, %u], stride=[%u, %u, %u], interlaced=%d.",
                 msg->alloc_param.width,
                 msg->alloc_param.height,
                 output->nplanes,
                 output->size[0],
                 output->size[1],
                 output->size[2],
                 output->stride[0],
                 output->stride[1],
                 output->stride[2],
                 output->interlaced);
        session->event(session, MVX_SESSION_EVENT_PORT_CHANGED,
                        (void *)MVX_DIR_OUTPUT);
        break;
    }
    case MVX_FW_CODE_BUFFER_GENERAL: {
        struct mvx_buffer *buf = msg->buf;
        if(buf->dir == MVX_DIR_INPUT)
        {
            session->port[buf->dir].buffer_count--;
            session->event(session, MVX_SESSION_EVENT_BUFFER, buf);
        }
        else
        {
            if( mvx_is_frame(buf->format))
            {
                if(buf->general.header.type ==MVX_BUFFER_GENERAL_TYPE_AD_STATS)
                {
                     MVX_SESSION_INFO(session,
                     "Firmware rsp: Buffer GENERAL. dir=%u, type=%u,frame_averages =0x%x thumbnail w/h=[%u, %u], ad_stats_flags=%x",
                     buf->dir,
                     buf->general.header.type,
                     buf->general.config.ad_stats.frame_averages,
                     buf->general.config.ad_stats.thumbnail_width,
                     buf->general.config.ad_stats.thumbnail_height,
                     buf->general.config.ad_stats.ad_stats_flags);
                }
            }
        }
        break;
    }
    case MVX_FW_CODE_BUFFER: {
        struct mvx_buffer *buf = msg->buf;
        struct mvx_session_port *output =
            &session->port[MVX_DIR_OUTPUT];

        /*
         * There is no point to flush or invalidate input buffer
         * after it was returned from the HW.
         */
        if(buf->dir == MVX_DIR_OUTPUT && mvx_is_frame(buf->format)) {
            if (!(buf->flags & MVX_BUFFER_FRAME_PRESENT)) {
                int i=0;
                for(i=0;i<buf->nplanes;i++)
                {
                    if (output->size[i] > mvx_buffer_size(buf, i)
                        ||    session->port[buf->dir].buffer_allocated < session->port[buf->dir].buffer_min)
                    {
                        buf->flags |= MVX_BUFFER_FRAME_NEED_REALLOC;
                        //output->isreallocting = true;
                    }
                }
            }
        }
        if (buf->dir == MVX_DIR_OUTPUT && session->port[MVX_DIR_INPUT].format == MVX_FORMAT_AV1 &&
            (buf->width != output->width || buf->height != output->height))
        {
            uint32_t i;
            uint32_t filled[MVX_BUFFER_NPLANES];
            uint32_t stride[MVX_BUFFER_NPLANES];
            memset(filled,0,sizeof(filled));
            memset(stride,0,sizeof(stride));
            output->nplanes = 0;
            mvx_buffer_frame_dim(output->format, buf->width, buf->height, &output->nplanes,
                       stride, filled, session->setting_stride);
            for (i = 0; i < buf->nplanes; i++)
            {
                (void)mvx_buffer_filled_set(buf, i, filled[i], 0);
            }

            MVX_SESSION_INFO(session,
                 "Firmware rsp: Buffer. dir=%u, len=[%u, %u, %u], flags=0x%08x, eos=%u",
                 buf->dir,
                 filled[0],
                 filled[1],
                 filled[2],
                 buf->flags,
                 (buf->flags & MVX_BUFFER_EOS) != 0);
        }
        session->port[buf->dir].buffer_count--;

        MVX_SESSION_INFO(session,
                 "Firmware rsp: Buffer. dir=%u, len=[%u, %u, %u], flags=0x%08x, eos=%u",
                 buf->dir,
                 buf->planes[0].filled,
                 buf->planes[1].filled,
                 buf->planes[2].filled,
                 buf->flags,
                 (buf->flags & MVX_BUFFER_EOS) != 0);
        if (buf->dir == MVX_DIR_OUTPUT)
            mvx_buffer_synch(buf, DMA_FROM_DEVICE);

        session->event(session, MVX_SESSION_EVENT_BUFFER, buf);
        break;
    }
    case MVX_FW_CODE_DISPLAY_SIZE: {
        if (session->port[MVX_DIR_INPUT].format == MVX_FORMAT_AV1) {
            uint32_t stride[MVX_BUFFER_NPLANES];
            uint32_t i;
        struct mvx_session_port *output =
                &session->port[MVX_DIR_OUTPUT];

            output->nplanes = 0;
            memset(stride,0,sizeof(stride));
            mvx_buffer_frame_dim(output->format, msg->disp_size.display_width, msg->disp_size.display_height,
                &output->nplanes, stride, output->display_size, session->setting_stride);
            for (i = 0; i < MVX_BUFFER_NPLANES; i++)
            {
                session->setting_stride[i] = max_t(unsigned int, session->setting_stride[i], stride[i]);
            }
            MVX_SESSION_INFO(session,
                 "Firmware rsp: display size. len=[%u, %u, %u]",
                 output->display_size[0],
                 output->display_size[1],
                 output->display_size[2]);
        }
        break;
    }
    case MVX_FW_CODE_COLOR_DESC: {
        MVX_SESSION_INFO(session,
                 "Firmware rsp: Color desc.");
        session->color_desc = msg->color_desc;
        session->event(session, MVX_SESSION_EVENT_COLOR_DESC, NULL);
        break;
    }
    case MVX_FW_CODE_ERROR: {
        MVX_SESSION_WARN(session,
                 "Firmware rsp: Error. code=%u, message=%s.",
                 msg->error.error_code, msg->error.message);

        /*
         * Release the dev session. It will prevent a dead session from
         * blocking the scheduler.
         */
        watchdog_stop(session);
        session_unregister(session);
        send_event_error(session, -EINVAL);
        break;
    }
    case MVX_FW_CODE_FLUSH: {
        MVX_SESSION_INFO(session, "Firmware rsp: Flushed. dir=%d.",
                 msg->flush.dir);
        session->port[msg->flush.dir].is_flushing = false;
        session->port[msg->flush.dir].flushed = true;
        (void)queue_pending_buffers(session, msg->flush.dir);
        break;
    }
    case MVX_FW_CODE_IDLE: {
        int ret;
        struct mvx_fw_msg msg_ack;

        MVX_SESSION_INFO(session, "Firmware rsp: Idle.");

        session->idle_count++;

        if (session->idle_count == 2)
            fw_switch_out(session);

        msg_ack.code = MVX_FW_CODE_IDLE_ACK;
        ret = session->fw.ops.put_message(&session->fw, &msg_ack);
        if (ret == 0)
            ret = send_irq(session);

        if (ret != 0)
            send_event_error(session, ret);

        break;
    }
    case MVX_FW_CODE_JOB: {
        MVX_SESSION_INFO(session, "Firmware rsp: Job.");
        (void)fw_job(session, 1);
        break;
    }
    case MVX_FW_CODE_PONG:
        MVX_SESSION_INFO(session, "Firmware rsp: Pong.");
        break;
    case MVX_FW_CODE_SEQ_PARAM: {
        struct mvx_session_port *p = &session->port[MVX_DIR_OUTPUT];

        MVX_SESSION_INFO(session,
                 "Firmware rsp: Seq param. planar={buffers_min=%u}, afbc={buffers_min=%u}, interlaced=%d.",
                 msg->seq_param.planar.buffers_min,
                 msg->seq_param.afbc.buffers_min,
                 p->interlaced);

        if (mvx_is_afbc(p->format) != false)
            p->buffer_min = msg->seq_param.afbc.buffers_min;
        else
            p->buffer_min = msg->seq_param.planar.buffers_min;

        (void)fw_flush(session, MVX_DIR_OUTPUT);
        break;
    }
    case MVX_FW_CODE_SET_OPTION: {
        MVX_SESSION_INFO(session, "Firmware rsp: Set option.");
        break;
    }
    case MVX_FW_CODE_STATE_CHANGE: {
        MVX_SESSION_INFO(session,
                 "Firmware rsp: State changed. old=%s, new=%s.",
                 state_to_string(session->fw_state),
                 state_to_string(msg->state));
        session->fw_state = msg->state;
        break;
    }
    case MVX_FW_CODE_SWITCH_IN: {
        watchdog_start(session, session_watchdog_timeout);
        break;
    }
    case MVX_FW_CODE_SWITCH_OUT: {
        MVX_SESSION_INFO(session, "Firmware rsp: Switched out.");

        watchdog_stop(session);
        session->switched_in = false;

        if ((session->fw_state == MVX_FW_STATE_RUNNING &&
             session->idle_count < 2) ||
            session->fw.msg_pending > 0)
            switch_in(session);

        break;
    }
    case MVX_FW_CODE_DUMP:
        break;
    case MVX_FW_CODE_DEBUG:
        break;
    case MVX_FW_CODE_UNKNOWN: {
        print_debug(session);
        break;
    }
    case MVX_FW_CODE_MAX:
        break;
    default:
        MVX_SESSION_WARN(session, "Unknown fw msg code. code=%u.",
                 msg->code);
    }
}

void mvx_session_irq(struct mvx_if_session *isession)
{
    struct mvx_session *session = mvx_if_session_to_session(isession);
    int ret;

    if (is_fw_loaded(session) == false)
        return;

    ret = session->fw.ops.handle_rpc(&session->fw);
    if (ret < 0) {
        send_event_error(session, ret);
        return;
    }

    do {
        struct mvx_fw_msg msg;

        watchdog_update(session, session_watchdog_timeout);

        ret = session->fw.ops.get_message(&session->fw, &msg);
        if (ret < 0) {
            session_unregister(session);
            send_event_error(session, ret);
            return;
        }

        if (ret > 0)
            handle_fw_message(session, &msg);
    } while (ret > 0 && session->error == 0);

    ret = session->fw.ops.handle_fw_ram_print(&session->fw);
    if (ret < 0) {
        send_event_error(session, ret);
        return;
    }

    wake_up(&session->waitq);
}

void mvx_session_port_show(struct mvx_session_port *port,
               struct seq_file *s)
{
    mvx_seq_printf(s, "mvx_session_port", 0, "%p\n", port);
    mvx_seq_printf(s, "format", 1, "%08x\n", port->format);
    mvx_seq_printf(s, "width", 1, "%u\n", port->width);
    mvx_seq_printf(s, "height", 1, "%u\n", port->height);
    mvx_seq_printf(s, "buffer_min", 1, "%u\n", port->buffer_min);
    mvx_seq_printf(s, "buffer_count", 1, "%u\n", port->buffer_count);
}

int mvx_session_set_securevideo(struct mvx_session *session,
                bool securevideo)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->isession.securevideo = securevideo;

    return 0;
}

int mvx_session_set_frame_rate(struct mvx_session *session,
                   int64_t frame_rate)
{
    int ret;
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false) {
        struct mvx_fw_set_option option;

        option.code = MVX_FW_SET_FRAME_RATE;
        option.frame_rate = frame_rate;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }

    session->frame_rate = frame_rate;

    return 0;
}

int mvx_session_set_rate_control(struct mvx_session *session,
                 bool enabled)
{
    if (session->error != 0)
        return session->error;

    session->rc_enabled = enabled;

    return 0;
}

int mvx_session_set_bitrate(struct mvx_session *session,
                int bitrate)
{
    int ret;

    if (session->error != 0)
        return session->error;

    session->target_bitrate = bitrate;

    if (is_fw_loaded(session) != false && session->rc_enabled != false) {
        struct mvx_fw_set_option option;

        option.code = MVX_FW_SET_RATE_CONTROL;
        option.rate_control.target_bitrate = session->target_bitrate;
        option.rate_control.rate_control_mode = session->rc_type;

        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }

    return 0;
}

int mvx_session_set_bitrate_control(struct mvx_session *session,
                struct mvx_buffer_param_rate_control *rc){
    int ret;

    if (session->error != 0)
        return session->error;

    session->rc_type = rc->rate_control_mode;
    session->target_bitrate = rc->target_bitrate;
    session->maximum_bitrate = rc->maximum_bitrate;
    if (is_fw_loaded(session) != false && session->port[get_bitstream_port(session)].format != MVX_FORMAT_JPEG) {
        struct mvx_fw_set_option option;

        option.code = MVX_FW_SET_RATE_CONTROL;
        option.rate_control.target_bitrate = rc->target_bitrate;
        option.rate_control.rate_control_mode = rc->rate_control_mode;
        if (rc->rate_control_mode == MVX_OPT_RATE_CONTROL_MODE_C_VARIABLE) {
            option.rate_control.maximum_bitrate = rc->maximum_bitrate;
        }
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }
    return 0;
}

int mvx_session_set_crop_left(struct mvx_session * session, int32_t left){

    if (session->error != 0)
        return session->error;

    session->crop_left = left;

    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;

}

int mvx_session_set_crop_right(struct mvx_session * session, int32_t right){

    if (session->error != 0)
        return session->error;

    session->crop_right = right;

    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;

}

int mvx_session_set_crop_top(struct mvx_session * session, int32_t top){

    if (session->error != 0)
        return session->error;

    session->crop_top = top;

    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;

}

int mvx_session_set_crop_bottom(struct mvx_session * session, int32_t bottom){

    if (session->error != 0)
        return session->error;

    session->crop_bottom = bottom;

    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;

}

int mvx_session_set_rc_bit_i_mode(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    session->rc_bit_i_mode = val;

    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;
}

int mvx_session_set_rc_bit_i_ratio(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    session->rc_bit_i_ratio = val;

    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;
}

int mvx_session_set_inter_med_buf_size(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    session->inter_med_buf_size = val;

    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;
}

int mvx_session_set_svct3_level1_period(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    session->svct3_level1_period = val;

    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;
}

int mvx_session_set_nalu_format(struct mvx_session *session,
                enum mvx_nalu_format fmt)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->nalu_format = fmt;

    return 0;
}

int mvx_session_set_stream_escaping(struct mvx_session *session,
                    enum mvx_tristate status)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->stream_escaping = status;

    return 0;
}

int mvx_session_set_profile(struct mvx_session *session,
                enum mvx_format format,
                enum mvx_profile profile)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->profile[format] = profile;

    return 0;
}

int mvx_session_set_level(struct mvx_session *session,
              enum mvx_format format,
              enum mvx_level level)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->level[format] = level;

    return 0;
}

int mvx_session_set_ignore_stream_headers(struct mvx_session *session,
                      enum mvx_tristate status)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->ignore_stream_headers = status;

    return 0;
}

int mvx_session_set_frame_reordering(struct mvx_session *session,
                     enum mvx_tristate status)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->frame_reordering = status;

    return 0;
}

int mvx_session_set_intbuf_size(struct mvx_session *session,
                int size)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->intbuf_size = size;

    return 0;
}

int mvx_session_set_p_frames(struct mvx_session *session,
                 int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->p_frames = val;

    return 0;
}

int mvx_session_set_b_frames(struct mvx_session *session,
                 int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->b_frames = val;

    return 0;
}

int mvx_session_set_gop_type(struct mvx_session *session,
                 enum mvx_gop_type gop_type)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->gop_type = gop_type;

    return 0;
}

int mvx_session_set_cyclic_intra_refresh_mb(struct mvx_session *session,
                        int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->cyclic_intra_refresh_mb = val;
    return 0;
}

int mvx_session_set_constr_ipred(struct mvx_session *session,
                 enum mvx_tristate status)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->constr_ipred = status;

    return 0;
}

int mvx_session_set_entropy_sync(struct mvx_session *session,
                 enum mvx_tristate status)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->entropy_sync = status;

    return 0;
}

int mvx_session_set_temporal_mvp(struct mvx_session *session,
                 enum mvx_tristate status)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->temporal_mvp = status;

    return 0;
}

int mvx_session_set_tile_rows(struct mvx_session *session,
                  int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->tile_rows = val;

    return 0;
}

int mvx_session_set_tile_cols(struct mvx_session *session,
                  int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->tile_cols = val;

    return 0;
}

int mvx_session_set_min_luma_cb_size(struct mvx_session *session,
                     int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;
    if (val == 8 || val == 16){
        session->min_luma_cb_size = val;
    } else {
        session->min_luma_cb_size = 0;
    }
    return 0;
}

int mvx_session_set_mb_mask(struct mvx_session *session,
                int val)
{
    /*
     * This controls is not implemented.
     */
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->mb_mask = val;

    return 0;
}

int mvx_session_set_entropy_mode(struct mvx_session *session,
                 enum mvx_entropy_mode mode)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->entropy_mode = mode;

    return 0;
}

int mvx_session_set_multi_slice_mode(struct mvx_session *session,
                     enum mvx_multi_slice_mode mode)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->multi_slice_mode = mode;

    return 0;
}

int mvx_session_set_multi_slice_max_mb(struct mvx_session *session,
                       int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->multi_slice_max_mb = val;

    return 0;
}

int mvx_session_set_vp9_prob_update(struct mvx_session *session,
                    enum mvx_vp9_prob_update mode)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->vp9_prob_update = mode;

    return 0;
}

int mvx_session_set_mv_h_search_range(struct mvx_session *session,
                      int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->mv_h_search_range = val;

    return 0;
}

int mvx_session_set_mv_v_search_range(struct mvx_session *session,
                      int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->mv_v_search_range = val;

    return 0;
}

int mvx_session_set_bitdepth_chroma(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->bitdepth_chroma = val;

    return 0;
}

int mvx_session_set_bitdepth_luma(struct mvx_session *session,
                  int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->bitdepth_luma = val;

    return 0;
}

int mvx_session_set_force_chroma_format(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->force_chroma_format = val;

    return 0;
}

int mvx_session_set_rgb_to_yuv_mode(struct mvx_session *session,
                    enum mvx_rgb_to_yuv_mode mode)
{
    if(mode == MVX_RGB_TO_YUV_MODE_MAX)
        return 0;
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->rgb_to_yuv = mode;
    session->use_cust_rgb_to_yuv_mode = MVX_CUST_YUV2RGB_MODE_STANDARD;

    return 0;
}

int mvx_session_set_band_limit(struct mvx_session *session,
                   int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->band_limit = val;

    return 0;
}

int mvx_session_set_cabac_init_idc(struct mvx_session *session,
                   int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->cabac_init_idc = val;

    return 0;
}

int mvx_session_set_i_frame_qp(struct mvx_session *session,
                   enum mvx_format fmt,
                   int qp)
{
    int ret;

    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false) {
        enum mvx_direction dir = get_bitstream_port(session);

        fmt = session->port[dir].format;
        ret = fw_set_qp(session, MVX_FW_SET_QP_I, qp);
        if (ret != 0)
            return ret;
    }

    session->qp[fmt].i_frame = qp;

    return 0;
}

int mvx_session_set_p_frame_qp(struct mvx_session *session,
                   enum mvx_format fmt,
                   int qp)
{
    int ret;

    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false) {
        enum mvx_direction dir = get_bitstream_port(session);

        fmt = session->port[dir].format;
        ret = fw_set_qp(session, MVX_FW_SET_QP_P, qp);
        if (ret != 0)
            return ret;
    }

    session->qp[fmt].p_frame = qp;

    return 0;
}

int mvx_session_set_b_frame_qp(struct mvx_session *session,
                   enum mvx_format fmt,
                   int qp)
{
    int ret;

    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false) {
        enum mvx_direction dir = get_bitstream_port(session);

        fmt = session->port[dir].format;
        ret = fw_set_qp(session, MVX_FW_SET_QP_B, qp);
        if (ret != 0)
            return ret;
    }

    session->qp[fmt].b_frame = qp;

    return 0;
}

int mvx_session_set_min_qp(struct mvx_session *session,
               enum mvx_format fmt,
               int qp)
{
    int ret;

    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false) {
        struct mvx_fw_set_option option;
        enum mvx_direction dir = get_bitstream_port(session);
        int codec = session->port[dir].format;

        option.code = MVX_FW_SET_QP_RANGE;
        option.qp_range.min = qp;
        option.qp_range.max = session->qp[codec].max;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }

    session->qp[fmt].min = qp;

    return 0;
}

int mvx_session_set_max_qp(struct mvx_session *session,
               enum mvx_format fmt,
               int qp)
{
    int ret;

    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false) {
        struct mvx_fw_set_option option;
        enum mvx_direction dir = get_bitstream_port(session);
        int codec = session->port[dir].format;

        option.code = MVX_FW_SET_QP_RANGE;
        option.qp_range.min = session->qp[codec].min;
        option.qp_range.max = qp;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }

    session->qp[fmt].max = qp;

    return 0;
}

int mvx_session_set_resync_interval(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->resync_interval = val;

    return 0;
}

int mvx_session_set_jpeg_quality(struct mvx_session *session,
                 int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->jpeg_quality = val;

    return 0;
}

int mvx_session_set_jpeg_quality_luma(struct mvx_session *session,
                 int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->jpeg_quality_luma = val;

    return 0;
}

int mvx_session_set_jpeg_quality_chroma(struct mvx_session *session,
                 int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->jpeg_quality_chroma = val;

    return 0;
}


int mvx_session_get_color_desc(struct mvx_session *session,
                   struct mvx_fw_color_desc *color_desc)
{
    *color_desc = session->color_desc;
    return 0;
}

int mvx_session_set_color_desc(struct mvx_session *session,
                   struct mvx_fw_color_desc *color_desc)
{
    int ret = 0;
    if (session->error != 0)
        return session->error;

    session->color_desc = *color_desc;
    if (is_fw_loaded(session) != false) {
        struct mvx_fw_set_option option;

        option.code = MVX_FW_SET_COLOUR_DESC;
        option.colour_desc = *color_desc;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }
    return 0;
}

int mvx_session_set_osd_config(struct mvx_session *session,
                   struct mvx_osd_config *osd)
{
    int ret = 0;
    int osd_cfg_num = 0;
    if (is_fw_loaded(session) == false ||
        session->port[MVX_DIR_INPUT].is_flushing != false) {
        osd_cfg_num = session->port[MVX_DIR_INPUT].osd_cfg_num;
        if (osd_cfg_num < MVX_ROI_QP_NUMS) {
            MVX_SESSION_INFO(session, "fw is not ready!!!, pending osd num:%d",osd_cfg_num);
            session->port[MVX_DIR_INPUT].osd_cfg_queue[osd_cfg_num] = *osd;
            session->port[MVX_DIR_INPUT].osd_cfg_num++;
        } else {
            MVX_SESSION_ERR(session, "fw is not ready for long time, too many osd pending:%d",osd_cfg_num);
        }
        return 0;
    }
    ret = queue_osd_config(session, osd);
    return ret;
}

int mvx_session_set_osd_info(struct mvx_session *session,
                   struct mvx_osd_info *osd)
{
    session->osd_info = *osd;
    return 0;
};

int mvx_session_set_roi_regions(struct mvx_session *session,
                   struct mvx_roi_config *roi)
{
    int ret = 0;
    int roi_config_num = 0;
    if (is_fw_loaded(session) == false ||
        session->port[MVX_DIR_INPUT].is_flushing != false) {
        roi_config_num = session->port[MVX_DIR_INPUT].roi_config_num;
        if (roi_config_num < MVX_ROI_QP_NUMS) {
            MVX_SESSION_INFO(session, "fw is not ready!!!, pending roi num:%d",roi_config_num);
            session->port[MVX_DIR_INPUT].roi_config_queue[roi_config_num] = *roi;
            session->port[MVX_DIR_INPUT].roi_config_num++;
        } else {
            MVX_SESSION_ERR(session, "fw is not ready for long time, too many roi pending:%d",roi_config_num);
        }
        return 0;
    }
    ret = queue_roi_regions(session, roi);
    return 0;
}

int mvx_session_set_qp_epr(struct mvx_session *session,
                   struct mvx_buffer_param_qp *qp)
{
    int ret = 0;
    int qp_num = 0;
    if (is_fw_loaded(session) == false ||
        session->port[MVX_DIR_INPUT].is_flushing != false) {
        qp_num = session->port[MVX_DIR_INPUT].qp_num;
        if (qp_num < MVX_ROI_QP_NUMS) {
            MVX_SESSION_WARN(session, "fw is not ready!!!, pending qp num:%d",qp_num);
            session->port[MVX_DIR_INPUT].qp_queue[qp_num] = *qp;
            session->port[MVX_DIR_INPUT].qp_num++;
        } else {
            MVX_SESSION_ERR(session, "fw is not ready for long time, too many qp pending:%d",qp_num);
        }
        return 0;
    }
    ret = queue_qp_epr(session, qp);
    return 0;
}

int mvx_session_set_sei_userdata(struct mvx_session *session,
                   struct mvx_sei_userdata *userdata)
{
    int ret = 0;
    if (session->error != 0)
        return session->error;

    session->sei_userdata = *userdata;
    if (is_fw_loaded(session) != false) {
        struct mvx_fw_set_option option;

        option.code = MVX_FW_SET_SEI_USERDATA;
        option.userdata = *userdata;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }
    return ret;
}

int mvx_session_set_hrd_buffer_size(struct mvx_session *session,
                  int size)
{
    int ret;

    if (session->error != 0)
        return session->error;

    session->nHRDBufsize = size;

    if (is_fw_loaded(session) != false) {
        struct mvx_fw_set_option option;

        option.code = MVX_FW_SET_HRD_BUF_SIZE;
        option.nHRDBufsize = size;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }
    return 0;
}

int mvx_session_set_dsl_frame(struct mvx_session *session,
                  struct mvx_dsl_frame *dsl)
{
    if (session->error != 0)
        return session->error;

    session->dsl_frame.width = dsl->width;
    session->dsl_frame.height = dsl->height;
    if (is_fw_loaded(session) != false) {
        return -EBUSY;
    }
    return 0;
}

int mvx_session_set_dsl_ratio(struct mvx_session *session,
                  struct mvx_dsl_ratio *dsl)
{
    if (session->error != 0)
        return session->error;

    session->dsl_ratio.hor = dsl->hor;
    session->dsl_ratio.ver = dsl->ver;
    return 0;
}

int mvx_session_set_long_term_ref(struct mvx_session *session,
                  struct mvx_long_term_ref *ltr)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false) {
         return -EBUSY;
     }

    session->mvx_ltr.mode = ltr->mode;
    session->mvx_ltr.period = ltr->period;
    return 0;
}

int mvx_session_set_dsl_mode(struct mvx_session *session,
                   int *mode)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->dsl_pos_mode = *mode;

    return 0;
}

int mvx_session_set_mini_frame_height(struct mvx_session *session,
                   int *height)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->mini_frame_height = *height;
    return 0;
}

int mvx_session_set_stats_mode(struct mvx_session *session,
                   struct mvx_enc_stats *stats)
{
    int ret = 0;
    int enc_stats_num = 0;
    if (is_fw_loaded(session) == false ||
        session->port[MVX_DIR_INPUT].is_flushing != false) {
        enc_stats_num = session->port[MVX_DIR_INPUT].enc_stats_num;
        if (enc_stats_num < MVX_ROI_QP_NUMS) {
            MVX_SESSION_INFO(session, "fw is not ready!!!, pending enc stats num:%d",enc_stats_num);
            session->port[MVX_DIR_INPUT].enc_stats_queue[enc_stats_num] = *stats;
            session->port[MVX_DIR_INPUT].enc_stats_num++;
        } else {
            MVX_SESSION_ERR(session, "fw is not ready for long time, too many enc stats pending:%d",enc_stats_num);
        }
        return 0;
    }
    ret = queue_enc_stats(session, stats);
    return ret;
}

int mvx_session_set_chr_cfg(struct mvx_session *session,
                   struct mvx_chr_cfg *chr_cfg)
{
    int ret = 0;
    int chr_cfg_num = 0;
    if (is_fw_loaded(session) == false ||
        session->port[MVX_DIR_INPUT].is_flushing != false) {
        chr_cfg_num = session->port[MVX_DIR_INPUT].chr_cfg_num;
        if (chr_cfg_num < MVX_ROI_QP_NUMS) {
            MVX_SESSION_INFO(session, "fw is not ready!!!, pending chr cfg num:%d",chr_cfg_num);
            session->port[MVX_DIR_INPUT].chr_cfg_queue[chr_cfg_num] = *chr_cfg;
            session->port[MVX_DIR_INPUT].chr_cfg_num++;
        } else {
            MVX_SESSION_ERR(session, "fw is not ready for long time, too many chr cfg pending:%d",chr_cfg_num);
        }
        return 0;
    }
    ret = queue_chr_cfg(session, chr_cfg);
    return ret;
}

int mvx_session_set_huff_table (struct mvx_session *session,
                   struct mvx_huff_table *table)
{
    if (is_fw_loaded(session) !=false)
        return -EBUSY;
    memcpy(&session->huff_table, table, sizeof(struct mvx_huff_table));
    return 0;
}

int mvx_session_set_seamless_target (struct mvx_session *session,
                   struct mvx_seamless_target *seamless)
{
    if (is_fw_loaded(session) !=false)
        return -EBUSY;
    memcpy(&session->seamless_target, seamless, sizeof(struct mvx_seamless_target));
    return 0;
}

int mvx_session_set_init_qp_i(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->init_qpi = val;

    return 0;
}

int mvx_session_set_init_qp_p(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->init_qpp = val;

    return 0;
}

int mvx_session_set_sao_luma(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->sao_luma = val;

    return 0;
}

int mvx_session_set_sao_chroma(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->sao_chroma = val;

    return 0;
}

int mvx_session_set_delta_I_P(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->qp_delta_i_p = val;

    return 0;
}

int mvx_session_set_ref_rb_eb(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->ref_rb_en = val;

    return 0;
}

int mvx_session_set_rc_clip_top(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->rc_qp_clip_top = val;

    return 0;
}

int mvx_session_set_rc_clip_bot(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->rc_qp_clip_bot = val;

    return 0;
}

int mvx_session_set_qpmap_clip_top(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->qpmap_qp_clip_top = val;

    return 0;
}

int mvx_session_set_qpmap_clip_bot(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->qpmap_qp_clip_bot = val;

    return 0;
}
int mvx_session_set_max_qp_i(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
    {
        int ret;
        struct mvx_fw_set_option option;
        option.code = MVX_FW_SET_QP_RANGE_I;
        option.qp_range.min = session->min_qp_i;
        option.qp_range.max = val;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }
    session->max_qp_i = val;

    return 0;

}

int mvx_session_set_min_qp_i(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
    {
        int ret;
        struct mvx_fw_set_option option;
        option.code = MVX_FW_SET_QP_RANGE_I;
        option.qp_range.min = val;
        option.qp_range.max = session->max_qp_i;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }

    session->min_qp_i = val;

    return 0;
}

int mvx_session_set_fixedqp(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->fixedqp = val;

    return 0;

}
int mvx_session_set_fw_profiling(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->profiling = val;

    return 0;

}

int mvx_session_set_visible_width(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    session->visible_width = val;

    return 0;

}

int mvx_session_set_visible_height(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    session->visible_height = val;

    return 0;

}

int mvx_session_set_gop_reset_pframes(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
    {
        int ret;
        struct mvx_fw_set_option option;
        option.code = MVX_FW_SET_GOP_PFRAMES;
        option.reset_gop_pframes = val;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }

    session->reset_gop_pframes = val;
    return 0;
}

int mvx_session_set_ltr_reset_period(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
    {
        int ret;
        struct mvx_fw_set_option option;
        option.code = MVX_FW_SET_LTR_PERIOD;
        option.reset_ltr_period = val;
        ret = fw_set_option(session, &option);
        if (ret != 0)
            return ret;
    }

    session->reset_ltr_period = val;
    return 0;
}

int mvx_session_set_gdr_number(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->gdr_number = val;

    return 0;
}

int mvx_session_set_gdr_period(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->gdr_period = val;

    return 0;
}

int mvx_session_set_multi_sps_pps(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->multi_sps_pps = val;

    return 0;
}

int mvx_session_set_enable_visual(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->enable_visual = val;

    return 0;
}

int mvx_session_set_adaptive_intra_block(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->adaptive_intra_block = val;

    return 0;

}

int mvx_session_set_scd_enable(struct mvx_session *session,
                                                int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->scd_enable = val;

    return 0;
}

int mvx_session_set_scd_percent(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->scd_percent = val;

    return 0;
}

int mvx_session_set_scd_threshold(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->scd_threshold = val;

    return 0;
}

int mvx_session_set_aq_ssim_en(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->aq_ssim_en = val;

    return 0;
}

int mvx_session_set_aq_neg_ratio(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->aq_neg_ratio = val;

    return 0;
}

int mvx_session_set_aq_pos_ratio(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->aq_pos_ratio = val;

    return 0;
}

int mvx_session_set_aq_qpdelta_lmt(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->aq_qpdelta_lmt = val;

    return 0;
}

int mvx_session_set_aq_init_frm_avg_svar(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->aq_init_frm_avg_svar = val;

    return 0;
}

int mvx_session_set_color_conversion(struct mvx_session *session,
                     enum mvx_yuv_to_rgb_mode mode)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;
    session->color_conv_mode = mode;
    session->use_cust_color_conv_coef=false;

    return 0;
}

int mvx_session_set_color_conversion_ceof(struct mvx_session *session,
                     struct mvx_color_conv_coef *conv_coef)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    memcpy(&session->color_conv_coef, conv_coef, sizeof(struct mvx_color_conv_coef));
    session->use_cust_color_conv_coef=true;

    return 0;
}

int mvx_session_set_rgb_conv_yuv_coef(struct mvx_session *session,
                     struct mvx_rgb2yuv_color_conv_coef *conv_coef)
{
    if (session->error != 0)
        return session->error;
    if (is_fw_loaded(session) != false)
        return -EBUSY;

    memcpy(&session->rgb2yuv_color_conv_coef, conv_coef, sizeof(struct mvx_rgb2yuv_color_conv_coef));
    session->use_cust_color_conv_coef=true;
    session->use_cust_rgb_to_yuv_mode = MVX_CUST_YUV2RGB_MODE_CUSTOMIZED;

    return 0;
}

int mvx_session_set_forced_uv_value(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->forced_uv_value = val;

    return 0;
}

int mvx_session_set_dsl_interpolation_mode(struct mvx_session *session,
                    int mode)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->dsl_interp_mode = mode;

    return 0;
}

int mvx_session_set_disabled_features(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->disabled_features = val;

    return 0;
}

int mvx_session_set_change_pos(struct mvx_session *session,
                    int val)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    session->change_pos = val;

    return 0;
}

int mvx_session_set_enc_src_crop(struct mvx_session *session,
                     struct mvx_crop_cfg *crop)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    memcpy(&session->enc_src_crop, crop, sizeof(struct mvx_crop_cfg));

    return 0;
}

int mvx_session_set_dec_dst_crop(struct mvx_session *session,
                     struct mvx_crop_cfg *crop)
{
    if (session->error != 0)
        return session->error;

    if (is_fw_loaded(session) != false)
        return -EBUSY;

    memcpy(&session->dec_dst_crop, crop, sizeof(struct mvx_crop_cfg));

    return 0;
}

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

#ifndef __MVX_PLAYER_H__
#define __MVX_PLAYER_H__

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <cmath>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <memory>
#include <vector>
#include <list>

#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <mutex>
#include <stdio.h>
#include <fstream>

#include <linux/videodev2.h>
#include "mvx-v4l2-controls.h"

#include "reader/read_util.h"
#include "reader/parser.h"
#include "reader/start_code_reader.h"

struct epr_config
{
    unsigned int pic_index;

    struct v4l2_buffer_general_block_configs   block_configs;
    struct v4l2_buffer_param_qp                qp;
    bool block_configs_present;
    bool qp_present;

    size_t bc_row_body_size;
    union
    {
        char* _bc_row_body_data;
        struct v4l2_buffer_general_rows_uncomp_body* uncomp;
    } bc_row_body;

    epr_config(const size_t size = 0)
    {
        pic_index = 0;
        qp.qp = 0;
        clear();
        allocate_bprf(size);
    };
    epr_config(const epr_config& other)
        : pic_index                       (other.pic_index),
          block_configs                   (other.block_configs),
          qp                              (other.qp),
          block_configs_present           (other.block_configs_present),
          qp_present                      (other.qp_present)
    {
        allocate_bprf(other.bc_row_body_size);

        if (other.bc_row_body_size > 0)
        {
            std::copy(other.bc_row_body._bc_row_body_data,
                      other.bc_row_body._bc_row_body_data + other.bc_row_body_size,
                      bc_row_body._bc_row_body_data);
        }
    };
    ~epr_config()
    {
        if (bc_row_body_size > 0)
        {
            delete [] bc_row_body._bc_row_body_data;
        }
    };
    epr_config& operator= (epr_config other)
    {
        swap(*this, other);
        return *this;
    };
    friend void swap(epr_config& a, epr_config& b)
    {
        using std::swap;

        swap(a.pic_index, b.pic_index);
        swap(a.block_configs, b.block_configs);
        swap(a.qp, b.qp);
        swap(a.block_configs_present, b.block_configs_present);
        swap(a.qp_present, b.qp_present);

        swap(a.bc_row_body_size, b.bc_row_body_size);
        swap(a.bc_row_body._bc_row_body_data, b.bc_row_body._bc_row_body_data);
    };
    void clear(void)
    {
        block_configs_present = false;
        qp_present = false;
    }

private:
    void allocate_bprf(size_t size)
    {
        bc_row_body_size = size;
        if (size > 0)
        {
            bc_row_body._bc_row_body_data = new char[size];
        }
        else
        {
            bc_row_body._bc_row_body_data = NULL;
        }
    };
};

struct v4l2_gop_config
{
    uint32_t gop_pic;
    uint32_t gop_pframes;
};

struct v4l2_reset_ltr_peroid_config
{
    unsigned int reset_trigger_pic;
    unsigned int reset_ltr_peroid;
};

struct v4l2_enc_stats_cfg
{
    unsigned int reset_pic;
    unsigned int reset_cfg;
};

typedef std::list<epr_config> v4l2_epr_list_t;
typedef std::list<v4l2_mvx_roi_regions> v4l2_roi_list_t;
typedef std::list<v4l2_mvx_chr_config> v4l2_chr_list_t;
typedef std::queue<v4l2_gop_config> gop_list_t;
typedef std::queue<v4l2_reset_ltr_peroid_config> ltr_list_t;
typedef std::queue<v4l2_enc_stats_cfg> enc_stats_list_t;
typedef std::list<v4l2_osd_config> v4l2_osd_list_t;

/****************************************************************************
 * Exception
 ****************************************************************************/
class Exception :
    public std::exception
{
public:
    Exception(const char *fmt, ...);
    Exception(const std::string &str);
    virtual ~Exception() throw();

    virtual const char *what() const throw();

private:
    char msg[100];
};

/****************************************************************************
 * Buffer
 ****************************************************************************/

class Buffer
{
public:
    Buffer(const v4l2_format &format);
    Buffer(v4l2_buffer &buf, int fd, const v4l2_format &format, enum v4l2_memory memory_type);
    virtual ~Buffer();

    v4l2_buffer &getBuffer();
    const v4l2_format &getFormat() const;
    void setCrop(const v4l2_crop &crop);
    const v4l2_crop &getCrop() const;
    void setSeamless(const struct v4l2_mvx_seamless_target &seamless);
    const v4l2_mvx_seamless_target &getSeamless()const;
    void setFrameProcessCount(int count);
    const int getFrameProcessCount()const;
    std::vector<iovec> getImageSize() const;
    std::vector<iovec> getBytesUsed() const;
    void getPackedBuffer(char* img, size_t nplanes, size_t dst_heights[], size_t dst_stride[], size_t src_stride[]);
    void setBytesUsed(std::vector<iovec> &iov);
    void clearBytesUsed();
    void resetVendorFlags();
    void setCodecConfig(bool codecConfig);
    void setTimeStamp(unsigned int timeUs);
    void setEndOfFrame(bool eof);
    void setEndOfStream(bool eos);
    void update(v4l2_buffer &buf);
    void setInterlaced(bool interlaced);
    void setTiled(bool tiled);
    void setRotation(int rotation);
    void setEncRotation(int rotation);
    void setMirror(int mirror);
    void setDownScale(int scale);
    void setEndOfSubFrame(bool eos);
    std::vector<iovec> convert10Bit();
    std::vector<iovec> _convert10Bit(unsigned short* ptr_y, unsigned short* ptr_uv, size_t size_y, size_t size_uv);
    void setRoiCfg(struct v4l2_mvx_roi_regions roi);
    void setChrCfg(struct v4l2_mvx_chr_config chr);
    void setGopResetCfg(struct v4l2_gop_config cfg);
    void setLtrResetCfg(struct v4l2_reset_ltr_peroid_config cfg);
    bool getRoiCfgflag() {return isRoiCfg;}
    bool getChrCfgflag() {return isChrCfg;}
    bool getGopResetCfgflag() {return isGopCfg;}
    bool getLtrResetCfgflag() {return isLtrCfg;}
    void setResetStatsMode(uint32_t pic_index, uint32_t cfg_mode){stats_pic_index = pic_index;stats_cfg_mode = cfg_mode;}
    uint32_t &getResetStatsMode(){return stats_cfg_mode;}
    uint32_t &getResetStatsPicIndex(){return stats_pic_index;}
    struct v4l2_mvx_roi_regions getRoiCfg() {return roi_cfg;};
    struct v4l2_mvx_chr_config getChrCfg() {return chr_cfg;}
    struct v4l2_gop_config getGopResetCfg() {return gop_cfg;}
    struct v4l2_reset_ltr_peroid_config getLtrResetCfg() {return ltr_cfg;}
    void setSuperblock(bool superblock);
    void setROIflag(bool roi_valid);
    void setChrflag(bool chr_valid);
    void setGopResetflag(bool valid);
    void setLtrResetflag(bool valid);
    void setOsdCfgEnable(bool enable);
    bool getOsdCfgEnable(){return isOsdCfg;}
    void setOsdCfg(struct v4l2_osd_config osd);
    struct v4l2_osd_config getOsdCfg(){return osd_cfg;}
    void setOsdBufferflag(uint32_t index);
    void setAdStatsGeneralBuffer(int ad_stats);
    void setEPRflag();
    void set_force_idr_flag(bool idr);
    void setQPofEPR(struct v4l2_buffer_param_qp data) {epr_qp = data;};
    struct v4l2_buffer_param_qp getQPofEPR(){return epr_qp;}
    bool isGeneralBuffer(){return (buf.flags & V4L2_BUF_FLAG_MVX_BUFFER_EPR) == V4L2_BUF_FLAG_MVX_BUFFER_EPR;};
    bool isOsdBuffer(){return buf.reserved2 & V4L2_BUF_FLAG_MVX_OSD_MASK;}
    void memoryMap(int fd);
    void memoryUnmap();
    size_t getLength(unsigned int plane);
    void dmaMemoryMap(int dma_fd, const unsigned int plane);
    void dmaMemoryunMap(void *p, const unsigned int plane);
    void setLength(const unsigned int length, const unsigned int plane);
    void setDmaFd(int fd, const unsigned int plane, const unsigned offset = 0);
    unsigned int getNumPlanes() const;
    void *getPlaneptr(unsigned i) {return ptr[i];}
private:

    void *ptr[VIDEO_MAX_PLANES];
    v4l2_buffer buf;
    v4l2_plane planes[VIDEO_MAX_PLANES];
    const v4l2_format &format;
    v4l2_crop crop;
    struct v4l2_mvx_seamless_target  seamless;
    int frames_processed;
    bool isRoiCfg;
    bool isChrCfg;
    bool isGopCfg;
    bool isLtrCfg;
    bool isOsdCfg;
    struct v4l2_mvx_roi_regions roi_cfg;
    struct v4l2_mvx_chr_config chr_cfg;
    struct v4l2_gop_config gop_cfg;
    struct v4l2_reset_ltr_peroid_config ltr_cfg;
    struct v4l2_buffer_param_enc_stats enc_stats_res;
    struct v4l2_buffer_param_qp epr_qp;
    struct v4l2_osd_config osd_cfg;
    enum v4l2_memory memory_type;
    uint32_t stats_pic_index;
    uint32_t stats_cfg_mode;
};

/****************************************************************************
 * Input and output
 ****************************************************************************/

#pragma pack(push, 1)
class IVFHeader
{
public:
    IVFHeader();
    IVFHeader(uint32_t codec, uint16_t width, uint16_t height);

    uint32_t signature;
    uint16_t version;
    uint16_t length;
    uint32_t codec;
    uint16_t width;
    uint16_t height;
    uint32_t frameRate;
    uint32_t timeScale;
    uint32_t frameCount;
    uint32_t padding;

    static const uint32_t signatureDKIF;
};

class IVFFrame
{
public:
    IVFFrame();
    IVFFrame(uint32_t size, uint64_t timestamp);

    uint32_t size;
    uint64_t timestamp;
};

/* STRUCT_C (for details see specification SMPTE-421M) */
struct HeaderC
{
    uint32_t reserved : 28;
    uint32_t profile : 4;
};

/* Sequence Layer Data (for details see specification SMPTE-421M) */
class VC1SequenceLayerData
{
public:
    VC1SequenceLayerData();

    uint32_t numFrames : 24;
    uint8_t signature1;
    uint32_t signature2;
    uint32_t headerC;
    uint32_t restOfSLD[6];

    static const uint8_t magic1;
    static const uint32_t magic2;
};

/* Frame Layer Data (for details see specification SMPTE-421M) */
class VC1FrameLayerData
{
public:
    VC1FrameLayerData();

    uint32_t frameSize : 24;
    uint32_t reserved : 7;
    uint32_t key : 1;
    uint32_t timestamp;
    uint8_t data[];
};

class RVHeader
{
public:
    RVHeader();
    uint32_t metlen;
    uint32_t signature1;
    uint32_t signature2;

    static const uint32_t signatureVIDO;
    static const uint32_t signatureRV40;
    static const uint32_t signatureRV30;
};


class RVFrame
{
public:
    RVFrame();

    uint32_t size;
    uint32_t timestamp;
    uint16_t sequencenum;
    uint16_t flags;
    uint32_t lastpacket;
    uint32_t slicesnum;

    static const uint32_t slicestartcode;
};

class RVSlice
{
public:
    RVSlice();
    uint32_t startcode;
    uint32_t offset;
};

class AFBCHeader
{
public:
    AFBCHeader();
    AFBCHeader(const v4l2_format &format, size_t frameSize, const v4l2_crop &crop, bool tiled, const int field = FIELD_NONE);

    uint32_t magic;
    uint16_t headerSize;
    uint16_t version;
    uint32_t frameSize;
    uint8_t numComponents;
    uint8_t subsampling;
    uint8_t yuvTransform;
    uint8_t blockSplit;
    uint8_t yBits;
    uint8_t cbBits;
    uint8_t crBits;
    uint8_t alphaBits;
    uint16_t mbWidth;
    uint16_t mbHeight;
    uint16_t width;
    uint16_t height;
    uint8_t cropLeft;
    uint8_t cropTop;
    uint8_t param;
    uint8_t fileMessage;

    static const uint32_t MAGIC = 0x43424641;
    static const uint16_t VERSION = 5;//5;
    static const uint8_t PARAM_TILED_BODY = 0x00000001;
    static const uint8_t PARAM_TILED_HEADER = 0x00000002;
    static const uint8_t PARAM_32X8_SUPERBLOCK = 0x00000004;
    static const int FIELD_NONE = 0;
    static const int FIELD_TOP = 1;
    static const int FIELD_BOTTOM = 2;
};
#pragma pack(pop)

class IO
{
public:
    IO(uint32_t format, size_t width = 0, size_t height = 0, size_t strideAlign = 0);
    virtual ~IO() {}

    virtual void prepare(Buffer &buf) {}
    virtual void finalize(Buffer &buf) {}
    virtual bool eof() { return false; }
    virtual void setNaluFormat(int nalu){}
    virtual int getNaluFormat(){return 0;}
    virtual bool needDoubleCount(){return false;};

    uint32_t getFormat() const { return format; }
    uint8_t getProfile() const { return profile; }
    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    size_t getStrideAlign() const { return strideAlign; }
    int getDir(){return dir;}
    void setReadHeight(size_t h) {readHeight = h;}
    size_t getReadHeight() const { return readHeight;}
    void setRotation(int rot) {rotation = rot;}
    int getRotation() {return rotation;}


protected:
    uint32_t format;
    uint8_t profile;
    size_t width;
    size_t height;
    size_t strideAlign;
    size_t readHeight;
    int dir;//0 for input; 1 for output
    int rotation;
};

class Input :
    public IO
{
public:
    Input(uint32_t format, size_t width = 0, size_t height = 0, size_t strideAlign = 0);

    virtual void prepare(Buffer &buf) {}
    virtual void finalize(Buffer &buf) {}
    virtual void setNaluFormat(int nalu){}
    virtual int getNaluFormat(){return 0;}
    std::queue<int> idr_list;
};

class InputFile :
    public Input
{
public:
    InputFile(std::istream &input, uint32_t format);
    virtual ~InputFile();

    virtual void prepare(Buffer &buf);
    virtual bool eof();
    virtual void setNaluFormat(int nalu){naluFmt = nalu;}
    virtual int getNaluFormat(){return naluFmt;}
protected:
    InputFile(std::istream &input, uint32_t format, size_t width, size_t height, size_t strideAlign);
    std::istream &input;
    char* inputBuf;
    uint32_t offset;
    int state;
    int curlen;
    bool iseof;
    int naluFmt;
    uint32_t remaining_bytes;
    start_code_reader* reader;
    bool send_end_of_frame_flag;
    bool send_end_of_subframe_flag;
};

class InputIVF :
    public InputFile
{
public:
    InputIVF(std::istream &input, uint32_t informat);

    virtual void prepare(Buffer &buf);
    virtual bool eof();
protected:
    uint32_t left_bytes;
    uint64_t timestamp;
};

class InputRCV :
    public InputFile
{
public:
    InputRCV(std::istream &input);

    virtual void prepare(Buffer &buf);
    virtual bool eof();
private:
    bool codecConfigSent;
    VC1SequenceLayerData sld;
    uint32_t left_bytes;
    bool isRcv;
};

class InputRV :
    public InputFile
{
public:
    InputRV(std::istream &input);

    virtual void prepare(Buffer &buf);
    virtual bool eof();
    void prepare_config_data(Buffer &buf);
    void prepare_slice_data(Buffer &buf);


protected:
    RVHeader header;
    RVFrame  frameheader;
    uint32_t frame_left_bytes;
    uint32_t slice_read_indx;
    std::vector<RVSlice> slice;
    bool b_header_read;
};

class InputAFBC :
    public InputFile
{
public:
    InputAFBC(std::istream &input, uint32_t format, size_t width, size_t height);

    virtual void prepare(Buffer &buf);
    virtual bool eof();
};

class InputFileFrame :
    public InputFile
{
public:
    InputFileFrame(std::istream &input, uint32_t format, size_t width, size_t height, size_t strideAlign, size_t stride[] = {0});

    virtual void prepare(Buffer &buf);
    unsigned int get_prepared_frames(){return prepared_frames;}
    virtual bool eof();
    v4l2_chr_list_t *chr_list;
    gop_list_t *gop_list;
    ltr_list_t *ltr_list;
    enc_stats_list_t *enc_stats_list;
protected:
    size_t nplanes;
    size_t stride[3];
    size_t size[3];
    size_t heights[3];
    size_t framesize;
    v4l2_chr_list_t::iterator chr_cur;
    unsigned int prepared_frames;
};

class InputFileMiniFrame :
    public InputFileFrame

{
public:
    InputFileMiniFrame(std::istream &input, uint32_t format, size_t width, size_t height, size_t strideAlign, uint32_t mini_height, size_t stride[]);
    virtual void prepare(Buffer &buf);
    virtual bool eof();
protected:
    size_t offset[3];
    bool is_done[3];//one whoel frame is read done of each plane.
    int count;//nbr of frame
    uint32_t cnt_of_miniframe;
    uint32_t miniframe_height;
};

class InputFileFrameWithROI :
    public InputFileFrame
{
public:
    InputFileFrameWithROI(std::istream &input, uint32_t format,
                        size_t width, size_t height, size_t strideAlign, std::istream &roi, size_t stride[]);
    virtual void prepare(Buffer &buf);
    virtual ~InputFileFrameWithROI();
private:
    void load_roi_cfg();
    std::istream &roi_is;
    v4l2_roi_list_t *roi_list;
    v4l2_roi_list_t::iterator cur;
};

class InputFileFrameWithEPR :
    public InputFileFrame
{
public:
    InputFileFrameWithEPR(std::istream &input, uint32_t format,
                        size_t width, size_t height, size_t strideAlign, std::istream &epr, uint32_t oformat, size_t stride[]);
    virtual ~InputFileFrameWithEPR();
    virtual void prepare(Buffer &buf);
    void prepareEPR(Buffer &buf);
    virtual bool needDoubleCount(){return true;};
private:
    std::istream &epr_is;
    v4l2_epr_list_t *epr_list;
    v4l2_epr_list_t::iterator cur;
    uint32_t outformat;
    void load_epr_cfg();
    void read_efp_cfg(char *buf, int num_epr, struct epr_config *config);
    void read_row_cfg(char *buf, int row, int len, struct epr_config &config);
    void erp_adjust_bpr_to_64_64(
                                    struct v4l2_buffer_general_rows_uncomp_body* uncomp_body,
                                    int qp_delta,
                                    uint32_t bpr_base_idx,
                                    uint32_t row_off,
                                    uint8_t force,
                                    uint8_t quad_skip,
                                    int local_base);
};

class InputFileOsd
{
public:
    InputFileOsd(const char  * filename, uint32_t format, size_t width, size_t height, size_t strideAlign, size_t stride[] = {0});
    virtual void prepare(Buffer &buf);
    virtual bool eof();
protected:
    size_t nplanes;
    size_t stride[3];
    size_t size[3];
    size_t heights[3];
    size_t framesize;
    std::ifstream osd_is;
};

class InputFileFrameOSD :
    public InputFileFrame
{
public:
    InputFileFrameOSD(std::istream &input, uint32_t format, size_t width, size_t height, size_t strideAlign, size_t stride[] = {0});
    virtual void prepare(Buffer &buf);
    v4l2_osd_list_t *osd_list;
    v4l2_osd_list_t::iterator osd_cur;
    InputFileOsd* osd_file_1;
    InputFileOsd* osd_file_2;
private:
    uint32_t refresh_index;
};

class InputFrame :
    public Input
{
public:
    InputFrame(uint32_t format, size_t width, size_t height,
               size_t strideAlign, size_t nframes);

    virtual void prepare(Buffer &buf);
    virtual bool eof();

private:
    void rgb2yuv(unsigned int yuv[3], const unsigned int rgb[3]);

    size_t nplanes;
    size_t stride[3];
    size_t size[3];
    size_t nframes;
    size_t count;
    size_t heights[3];
};

class Output :
    public IO
{
public:
    Output(uint32_t format);
    Output(uint32_t format, bool packed);
    virtual ~Output();

    virtual void prepare(Buffer &buf);
    virtual void finalize(Buffer &buf);
    virtual void write(void *ptr, size_t nbytes) {}

protected:
    unsigned int timestamp;
    size_t totalSize;
    bool packed;
};

class OutputFile :
    public Output
{
public:
    OutputFile(std::ostream &output, uint32_t format);
    OutputFile(std::ostream &output, uint32_t format, bool packed);

    virtual void write(void *ptr, size_t nbytes);

protected:
    std::ostream &output;
};

class OutputIVF :
    public OutputFile
{
public:
    OutputIVF(std::ofstream &output, uint32_t format, uint16_t width, uint16_t height);

    virtual void finalize(Buffer &buf);

private:
    std::vector<char> temp;
};

class OutputAFBC :
    public OutputFile
{
public:
    OutputAFBC(std::ofstream &output, uint32_t format, bool tiled);
    virtual void prepare(Buffer &buf);
    virtual void finalize(Buffer &buf);
protected:
    bool tiled;
};

class OutputAFBCInterlaced :
    public OutputAFBC
{
public:
    OutputAFBCInterlaced(std::ofstream &output, uint32_t format, bool tiled);
    virtual void finalize(Buffer &buf);
};

class OutputFileWithMD5 :
    public OutputFile
{
public:
    OutputFileWithMD5(std::ofstream &output, uint32_t format, std::ofstream &output_md5, bool packed);
    virtual void finalize(Buffer &buf);

private:
    std::ofstream &output_md5;
};

class OutputFileWithADStats :
    public OutputFile
{
public:
    OutputFileWithADStats(std::ofstream &output, uint32_t format,
                                const std::string& ad_stats_filename,const std::string&thumbnail_filename, bool packed);
    virtual void finalize(Buffer &buf);
    virtual ~OutputFileWithADStats();
private:
    std::ofstream output_ad_stats;
    std::ofstream output_thumbnail;
};


class OutputFileFrameStats :
    public OutputFile
{
public:
    OutputFileFrameStats(std::ostream &output, uint32_t format, uint32_t stats_mode,
                        const std::string& filename, uint32_t width, uint32_t height);
    virtual void finalize(Buffer &buf);
    virtual ~OutputFileFrameStats();
private:
    uint32_t outformat;
    int enc_stats_mode;
    std::ofstream file_mms;
    std::ofstream file_bitcost;
    std::ofstream file_qp;
    unsigned int queued_buffer;
};

/****************************************************************************
 * Codec, Decoder, Encoder
 ****************************************************************************/

class Codec
{
public:
    typedef std::map<uint32_t, Buffer *> BufferMap;

    Codec(const char *dev,
          enum v4l2_buf_type inputType,
          enum v4l2_buf_type outputType,
          std::ostream &log,
          bool nonblock);
    Codec(const char *dev,
          Input &input,
          enum v4l2_buf_type inputType,
          Output &output,
          enum v4l2_buf_type outputType,
          std::ostream &log,
          bool nonblock);
    virtual ~Codec();

    int stream();
    void setMemoryType(enum v4l2_memory mem_type);
    void setInputThread(int input_thread);
    static uint32_t to4cc(const std::string &str);
    static bool isVPx(uint32_t format);
    static bool isYUV422(uint32_t format);
    static bool isAFBC(uint32_t format);
    static void getStride(uint32_t format, size_t & nplanes, size_t stride[3][2]);
    static size_t getSize(uint32_t format, size_t width, size_t height,
                          size_t strideAlign, size_t & nplanes, size_t stride[3], size_t size[3], size_t heights[3]);
protected:
    enum NaluFormat
    {
        NALU_FORMAT_START_CODES,
        NALU_FORMAT_ONE_NALU_PER_BUFFER,
        NALU_FORMAT_ONE_BYTE_LENGTH_FIELD,
        NALU_FORMAT_TWO_BYTE_LENGTH_FIELD,
        NALU_FORMAT_FOUR_BYTE_LENGTH_FIELD
    };

    class Port
    {
    public:
        Port(int &fd, enum v4l2_buf_type type, std::ostream &log) :
            fd(fd),
            type(type),
            log(log),
            interlaced(false),
            tryEncStop(false),
            tryDecStop(false),
            ad_stats(0),
            mirror(0),
            scale(1),
            frames_processed(0),
            frames_count(0),
            memory_type_port(V4L2_MEMORY_MMAP),
            isInputThread(false)
        {memset(__stride, 0, sizeof(size_t) * VIDEO_MAX_PLANES);
        memset(&seamless,0,sizeof(seamless));}
        Port(int &fd, IO &io, v4l2_buf_type type, std::ostream &log) :
            fd(fd),
            io(&io),
            type(type),
            log(log),
            pending(0),
            tid(0),
            interlaced(false),
            tryEncStop(false),
            tryDecStop(false),
            ad_stats(0),
            mirror(0),
            scale(1),
            frames_processed(0),
            frames_count(0),
            memory_type_port(V4L2_MEMORY_MMAP),
            isInputThread(false)
        {memset(__stride, 0, sizeof(size_t) * VIDEO_MAX_PLANES);
        memset(&seamless,0,sizeof(seamless));}

        void enumerateFormats();
        const v4l2_format &getFormat();
        void tryFormat(v4l2_format &format);
        void setFormat(v4l2_format &format);
        void getTrySetFormat();
        void printFormat(const struct v4l2_format &format);
        const v4l2_crop getCrop();

        void allocateBuffers(size_t count, enum v4l2_memory mempry_type);
        void freeBuffers();
        unsigned int getBufferCount();
        void queueBuffers();
        void queueBuffer(Buffer &buf);
        Buffer &dequeueBuffer();
        void printBuffer(const v4l2_buffer &buf, const char *prefix);

        bool handleBuffer();
        void handleResolutionChange();

        void streamon();
        void streamoff();

        void sendEncStopCommand();
        void sendDecStopCommand();

        void setH264DecIntBufSize(uint32_t ibs);
        void setNALU(NaluFormat nalu);
        void setEncFramerate(uint32_t fps);
        void setEncBitrate(uint32_t bps);
        void setEncPFrames(uint32_t pframes);
        void setEncBFrames(uint32_t bframes);
        void setEncSliceSpacing(uint32_t spacing);
        void setEncForceChroma(uint32_t fmt);
        void setEncBitdepth(uint32_t bd);
        void setH264EncIntraMBRefresh(uint32_t period);
        void setEncProfile(uint32_t profile);
        void setEncLevel(uint32_t level);
        void setEncConstrainedIntraPred(uint32_t cip);
        void setH264EncEntropyMode(uint32_t ecm);
        void setH264EncGOPType(uint32_t gop);
        void setEncMinQP(uint32_t minqp);
        void setEncMaxQP(uint32_t maxqp);
        void setEncFixedQP(uint32_t fqp);
        void setEncFixedQPI(uint32_t fqp);
        void setEncFixedQPP(uint32_t fqp);
        void setEncFixedQPB(uint32_t fqp);
        void setEncMinQPI(uint32_t nQpMinI);
        void setEncMaxQPI(uint32_t nQpMaxI);
        void setEncInitQPI(uint32_t init_qpi);
        void setEncInitQPP(uint32_t init_qpp);
        void setEncSAOluma(uint32_t sao_luma_dis);
        void setEncSAOchroma(uint32_t sao_chroma_dis);
        void setEncQPDeltaIP(uint32_t qp_delta_i_p);
        void setEncRefRbEn(uint32_t ref_rb_en);
        void setEncRCClipTop(uint32_t rc_qp_clip_top);
        void setEncRCClipBot(uint32_t rc_qp_clip_bottom);
        void setEncQpmapClipTop(uint32_t qpmap_clip_top);
        void setEncQpmapClipBot(uint32_t qpmap_clip_bottom);
        void setPortProfiling(uint32_t enable);
        void setH264EncBandwidth(uint32_t bw);
        void setHEVCEncEntropySync(uint32_t es);
        void setHEVCEncTemporalMVP(uint32_t tmvp);
        void setEncStreamEscaping(uint32_t sesc);
        void setEncHorizontalMVSearchRange(uint32_t hmvsr);
        void setEncVerticalMVSearchRange(uint32_t vmvsr);
        void setVP9EncTileCR(uint32_t tcr);
        void setJPEGEncQuality(uint32_t q);
        void setJPEGEncQualityLuma(uint32_t q);
        void setJPEGEncQualityChroma(uint32_t q);
        void setJPEGEncRefreshInterval(uint32_t r);
        void setJPEGHufftable(struct v4l2_mvx_huff_table *table);
        void setSeamlessTarget(struct v4l2_mvx_seamless_target * seamless);
        void setInterlaced(bool interlaced);
        void setRotation(int rotation);
        void setMirror(int mirror);
        void setDownScale(int scale);
        void tryEncStopCmd(bool tryStop);
        void tryDecStopCmd(bool tryStop);
        void setDecFrameReOrdering(uint32_t fro);
        void setDecIgnoreStreamHeaders(uint32_t ish);
        bool isEncoder();
        void setFrameCount(int frames);
        void setRateControl(struct v4l2_rate_control *rc);
        void setCropLeft(int left);
        void setCropRight(int right);
        void setCropTop(int top);
        void setColorConversion(uint32_t mode);
        void setCustColorConvCoef(struct v4l2_mvx_color_conv_coef *coef);
        void setCropBottom(int bottom);
        void setStatsMode(int mode, int index = 0);
        void setVuiColourDesc(struct v4l2_mvx_color_desc *color);
        void setSeiUserData(struct v4l2_sei_user_data *sei_user_data);
        void setHRDBufferSize(int size);
        void setDSLFrame(int width, int height);
        void setDSLRatio(int hor, int ver);
        void setLongTermRef(uint32_t mode, uint32_t period);
        void setDSLMode(int mode);
        void setDSLInterpMode(int mode);
        void setDisabledFeatures(int val);
        void setRGBToYUVMode(uint32_t mode);
        void setRGBConvertYUV(struct v4l2_mvx_rgb2yuv_color_conv_coef *coef);
        void setDecDstCrop(struct v4l2_mvx_crop_cfg *dst_crop);
        void setVisibleWidth(uint32_t v_width);
        void setVisibleHeight(uint32_t v_height);
        void setMiniHeight(uint32_t mini_height);
        int allocateDMABuf(size_t size);
        void setPortMemoryType(enum v4l2_memory mem_type);
        void startInputThread();
        static void* fillInputThread(void *arg);
        void _fillInputThread();
        uint32_t getInputBufferIdx(pthread_mutex_t *mutex, pthread_cond_t *cond, std::queue<uint32_t> *input_queue);
        void appendInputBufferIdx(pthread_mutex_t *mutex, pthread_cond_t *cond, std::queue<uint32_t> *input_queue, uint32_t index);
        void setFrameStride(size_t *stride);
        void setRcBitIMode(uint32_t mode);
        void setRcBitRationI(uint32_t ratio);
        void setMultiSPSPPS(uint32_t sps_pps);
        void setEnableVisual(uint32_t enable);
        void setEnableSCD(uint32_t scd_enable);
        void setScdPercent(uint32_t scd_percent);
        void setScdThreshold(uint32_t scd_threshold);
        void setEnableAQSsim(uint32_t aq_ssim_en);
        void setAQNegRatio(uint32_t aq_neg_ratio);
        void setAQPosRatio(uint32_t aq_pos_ratio);
        void setAQQPDeltaLmt(uint32_t aq_qpdelta_lmt);
        void setAQInitFrmAvgSvar(uint32_t aq_init_frm_avg_svar);
        void setAdaptiveIntraBlock(uint32_t enable);
        void setIntermediateBufSize(uint32_t size);
        void setSvct3Level1Period(uint32_t period);
        void setGopResetPframes(int pframes);
        void setLtrResetPeriod(int period);
        void setPortStatsSize(uint32_t mms, uint32_t bc, uint32_t qp);
        void setGDRnumber(uint32_t numbder);
        void setGDRperiod(uint32_t period);
        void setForcedUVvalue(uint32_t uv_value);
        void setEncSrcCrop(struct v4l2_mvx_crop_cfg * src_crop);
        void setBufferCnt(uint32_t count){buf_cnt = count;}
        uint32_t &getBufferCnt(){return buf_cnt;}//this buf_cnt comes from setting value, default 0
        void setOsdCfg(struct v4l2_osd_config osd);
        void setEncOSDinfo(struct v4l2_osd_info* info);
        void setChangePos(uint32_t pos);
        void setAdStats(int ad_stats);
        int &fd;
        IO *io;
        v4l2_buf_type type;
        v4l2_format format;
        std::ostream &log;
        BufferMap buffers;
        size_t pending;
        pthread_t tid;
        FILE *roi_cfg;

    private:
        int rotation;
        bool interlaced;
        bool tryEncStop;
        bool tryDecStop;
        int ad_stats;
        int mirror;
        int scale;
        int frames_processed;
        int frames_count;
        int rc_type;
        uint32_t mini_frame_height;
        enum v4l2_memory memory_type_port;
        bool isInputThread;
        std::queue<uint32_t> input_producer_queue;
        std::queue<uint32_t> input_consumer_queue;
        pthread_mutex_t input_producer_mutex;
        pthread_mutex_t input_consumer_mutex;
        pthread_cond_t input_producer_cond;
        pthread_cond_t input_consumer_cond;
        size_t __stride[VIDEO_MAX_PLANES];
        uint32_t mms_buffer_size;
        uint32_t bitcost_buffer_size;
        uint32_t qp_buffer_size;
        uint32_t buf_cnt;//buffer number to be allocated for this port
        struct v4l2_mvx_seamless_target seamless;
    };

    static size_t getBytesUsed(v4l2_buffer &buf);
    void enumerateFormats();

    Port input;
    Port output;
    int fd;
    std::ostream &log;
    bool csweo;
    uint32_t fps;
    uint32_t bps;
    uint32_t minqp;
    uint32_t maxqp;
    uint32_t fixedqp;
    uint32_t mini_frame_height;
    enum v4l2_memory memory_type;

private:
    void openDev(const char *dev);
    void closeDev();

    void queryCapabilities();
    void enumerateFramesizes(uint32_t format);
    void setFormats();

    v4l2_mvx_color_desc getColorDesc();
    void printColorDesc(const v4l2_mvx_color_desc &color);

    void subscribeEvents();
    void subscribeEvents(uint32_t event);
    void unsubscribeEvents();
    void unsubscribeEvents(uint32_t event);

    void allocateBuffers(enum v4l2_memory m_type = V4L2_MEMORY_MMAP);
    void freeBuffers();
    void queueBuffers();

    void streamon();
    void streamoff();

    void runPoll();
    void runThreads();
    static void *runThreadInput(void *arg);
    static void *runThreadOutput(void *arg);
    bool handleEvent();

    bool nonblock;
};

class Decoder :
    public Codec
{
public:
    Decoder(const char *dev, Input &input, Output &output, bool nonblock = true, std::ostream &log = std::cout);
    void setH264IntBufSize(uint32_t ibs);
    void setInterlaced(bool interlaced);
    void setFrameReOrdering(uint32_t fro);
    void setIgnoreStreamHeaders(uint32_t ish);
    void tryStopCmd(bool tryStop);
    void setNaluFormat(int nalu);
    void setRotation(int rotation);
    void setDownScale(int scale);
    void setFrameCount(int frames);
    void setDSLFrame(int width, int height);
    void setDSLRatio(int hor, int ver);
    void setDSLMode(int mode);
    void setDSLInterpMode(int mode);
    void setDisabledFeatures(int val);
    void setColorConversion(uint32_t mode);
    void setCustColorConvCoef(struct v4l2_mvx_color_conv_coef *coef);
    void setDecDstCrop(struct v4l2_mvx_crop_cfg *dst_crop);
    void setStride(size_t *stride);
    void setSeamlessTarget(uint32_t format, struct v4l2_mvx_seamless_target *seamless);
    void setFrameBufCnt(uint32_t count);
    void setBitBufCnt(uint32_t count);
    void setAdStats(int ad_stats);
private:
    int naluFmt;
};

class Encoder :
    public Codec
{
public:
    Encoder(const char *dev, Input &input, Output &output, bool nonblock = true, std::ostream &log = std::cout);
    void changeSWEO(uint32_t csweo);
    void setFramerate(uint32_t fps);
    void setBitrate(uint32_t bps);
    void setPFrames(uint32_t pframes);
    void setBFrames(uint32_t bframes);
    void setSliceSpacing(uint32_t spacing);
    void setConstrainedIntraPred(uint32_t cip);
    void setEncForceChroma(uint32_t fmt);
    void setEncBitdepth(uint32_t bd);
    void setH264IntraMBRefresh(uint32_t period);
    void setProfile(uint32_t profile);
    void setLevel(uint32_t level);
    void setH264EntropyCodingMode(uint32_t ecm);
    void setH264GOPType(uint32_t gop);
    void setEncMinQP(uint32_t minqp);
    void setEncMaxQP(uint32_t maxqp);
    void setEncFixedQP(uint32_t fqp);
    void setEncFixedQPI(uint32_t fqp);
    void setEncFixedQPP(uint32_t fqp);
    void setEncFixedQPB(uint32_t fqp);
    void setH264Bandwidth(uint32_t bw);
    void setVP9TileCR(uint32_t tcr);
    void setJPEGRefreshInterval(uint32_t ri);
    void setJPEGQuality(uint32_t q);
    void setJPEGQualityLuma(uint32_t q);
    void setJPEGQualityChroma(uint32_t q);
    void setJPEGHufftable(struct v4l2_mvx_huff_table *table);
    void setHEVCEntropySync(uint32_t es);
    void setHEVCTemporalMVP(uint32_t tmvp);
    void setStreamEscaping(uint32_t sesc);
    void setHorizontalMVSearchRange(uint32_t hmvsr);
    void setVerticalMVSearchRange(uint32_t vmvsr);
    void tryStopCmd(bool tryStop);
    void setMirror(int mirror);
    void setRotation(int rotation);
    void setFrameCount(int frames);
    void setRateControl(const std::string &rc, int target_bitrate, int maximum_bitrate);
    void setCropLeft(int left);
    void setCropRight(int right);
    void setCropTop(int top);
    void setCropBottom(int bottom);
    void setStatsMode(int mode);
    void setVuiColourDesc(struct v4l2_mvx_color_desc *color);
    void setSeiUserData(struct v4l2_sei_user_data *sei_user_data);
    void setHRDBufferSize(int size);
    void setLongTermRef(uint32_t mode, uint32_t period);
    void setMiniHeight(uint32_t mini_height);
    void setEncMinQPI(uint32_t nQpMinI);
    void setEncMaxQPI(uint32_t nQpMaxI);
    void setEncInitQPI(uint32_t init_qpi);
    void setEncInitQPP(uint32_t init_qpp);
    void setEncSAOluma(uint32_t sao_luma_dis);
    void setEncSAOchroma(uint32_t sao_chroma_dis);
    void setEncQPDeltaIP(uint32_t qp_delta_i_p);
    void setEncRefRbEn(uint32_t ref_rb_en);
    void setEncRCClipTop(uint32_t rc_qp_clip_top);
    void setEncRCClipBot(uint32_t rc_qp_clip_bottom);
    void setEncQpmapClipTop(uint32_t qpmap_clip_top);
    void setEncQpmapClipBot(uint32_t qpmap_clip_bottom);
    void setEncProfiling(uint32_t enable);
    void setVisibleWidth(uint32_t v_width);
    void setVisibleHeight(uint32_t v_height);
    void setStride(size_t *stride);
    void setRcBitIMode(uint32_t mode);
    void setRGBToYUVMode(uint32_t mode);
    void setRGBConvertYUV(struct v4l2_mvx_rgb2yuv_color_conv_coef *coef);
    void setRcBitRationI(uint32_t ratio);
    void setMultiSPSPPS(uint32_t sps_pps);
    void setEnableSCD(uint32_t scd_enable);
    void setScdPercent(uint32_t scd_percent);
    void setScdThreshold(uint32_t scd_threshold);
    void setEnableAQSsim(uint32_t aq_ssim_en);
    void setAQNegRatio(uint32_t aq_neg_ratio);
    void setAQPosRatio(uint32_t aq_pos_ratio);
    void setAQQPDeltaLmt(uint32_t aq_qpdelta_lmt);
    void setAQInitFrmAvgSvar(uint32_t aq_init_frm_avg_svar);
    void setEnableVisual(uint32_t enable);
    void setAdaptiveIntraBlock(uint32_t enable);
    void setIntermediateBufSize(uint32_t size);
    void setSvct3Level1Period(uint32_t period);
    void setStatsSize(uint32_t mms, uint32_t bc, uint32_t qp);
    void setGDRnumber(uint32_t numbder);
    void setGDRperiod(uint32_t period);
    void setForcedUVvalue(uint32_t uv_value);
    void setEncSrcCrop(struct v4l2_mvx_crop_cfg * src_crop);
    void setFrameBufCnt(uint32_t count);
    void setBitBufCnt(uint32_t count);
    void setEncOSDinfo(struct v4l2_osd_info* info);
    void setChangePos(uint32_t pos);
};

class Info :
    public Codec
{
public:
    Info(const char *dev, std::ostream &log = std::cout);
    void enumerateFormats();
};

#endif /* __MVX_PLAYER_H__ */

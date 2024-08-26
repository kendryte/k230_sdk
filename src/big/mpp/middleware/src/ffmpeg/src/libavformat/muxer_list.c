static const AVOutputFormat * const muxer_list[] = {
    &ff_adts_muxer,
    &ff_h264_muxer,
    &ff_hevc_muxer,
    &ff_mjpeg_muxer,
    &ff_mov_muxer,
    &ff_mp4_muxer,
    &ff_pcm_alaw_muxer,
    &ff_pcm_mulaw_muxer,
    &ff_rtp_muxer,
    &ff_rtsp_muxer,
    NULL };

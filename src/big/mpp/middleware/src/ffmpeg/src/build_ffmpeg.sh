# Get initial variables
export LDFLAGS="-T /home/sunxiaopeng/src/work/k230/k230_sdk/src/big/mpp/userapps/sample/linker_scripts/riscv64/link.lds -lpthread -Wl,--whole-archive -Wl,--no-whole-archive -n --static"
export RTT_EXEC_PATH=/home/sunxiaopeng/src/work/k230/k230_sdk/toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin
export PATH=$PATH:$RTT_EXEC_PATH
export CROSS_COMPILE="riscv64-unknown-linux-musl"
export CFLAGS="-mcmodel=medany -march=rv64imafdcv -mabi=lp64d -Wall -n --static"

# default build
function builddef() {
    ./configure \
    --cross-prefix=${CROSS_COMPILE} --enable-cross-compile --target-os=linux \
    --cc=${CROSS_COMPILE}-gcc \
    --ar=${CROSS_COMPILE}-ar \
    --nm=${CROSS_COMPILE}-nm \
    --ranlib=${CROSS_COMPILE}-ranlib \
    --arch=riscv64 --prefix=$(pwd)/lib_ffmpeg \
    --pkg-config-flags="--static" \
    --enable-gpl --extra-cflags="-fPIC -Wno-multichar -Wno-deprecated-declarations -Wno-unused-result -Wno-unused-variable -Wno-format -Wno-return-type -Wno-sign-compare -Wno-unused-label" --enable-nonfree --disable-ffplay --enable-swscale --enable-pthreads --disable-armv5te --disable-armv6 --disable-armv6t2 --disable-x86asm  --disable-stripping \
    --enable-libx264 --extra-cflags=-I/home/sunxiaopeng/src/work/k230/k230_sdk/src/big/mpp/middleware/src/x264/src/lib_x264/include --extra-ldflags=-L/home/sunxiaopeng/src/work/k230/k230_sdk/src/big/mpp/middleware/src/x264/src/lib_x264/lib --extra-libs=-ldl \
    --disable-debug --disable-doc --disable-htmlpages --disable-manpages --disable-podpages --disable-txtpages \
    --disable-encoders --enable-encoder=pcm_alaw  --enable-encoder=pcm_mulaw --enable-encoder=aac --enable-libx264 --enable-encoder=libx264 --enable-encoder=mjpeg \
    --disable-decoders --enable-decoder=h264 --enable-decoder=mjpeg --enable-decoder=hevc --enable-decoder=aac --enable-decoder=pcm_alaw --enable-decoder=pcm_mulaw \
    --disable-demuxers --enable-demuxer=h264 --enable-demuxer=hevc --enable-demuxer=mjpeg --enable-demuxer=aac --enable-demuxer=mp3 --enable-demuxer=pcm_alaw --enable-demuxer=pcm_mulaw --enable-demuxer=wav --enable-demuxer=rtp --enable-demuxer=rtsp \
    --disable-muxers --enable-muxer=mp4 --enable-muxer=adts --enable-muxer=h264 --enable-muxer=hevc --enable-muxer=mjpeg --enable-muxer=rtp --enable-muxer=rtsp --enable-muxer=pcm_alaw --enable-muxer=pcm_mulaw --enable-demuxer=wav \
    --disable-protocols --enable-protocol=file  --enable-protocol=rtp --enable-protocol=rtmp --enable-protocol=udp  --enable-protocol=tcp --enable-protocol=http --enable-protocol=hls \
    --disable-bsfs --enable-bsf=h264_mp4toannexb --enable-bsf=hevc_mp4toannexb --enable-bsf=aac_adtstoasc \
    --disable-parsers --enable-parser=aac --enable-parser=h264 --enable-parser=hevc --enable-parser=mjpeg --enable-parser=bmp --enable-parser=png \
    --disable-avdevice --disable-swscale --disable-iconv --disable-avfilter
    make clean
    if [ "$1" == "verbose" ]; then
        make V=1
    else
        make -j8
    fi
    make install
}
builddef $1
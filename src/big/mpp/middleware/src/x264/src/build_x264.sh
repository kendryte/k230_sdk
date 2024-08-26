# Get initial variables
export RTT_EXEC_PATH=/home/sunxiaopeng/src/work/k230/k230_sdk/toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin
export PATH=$PATH:$RTT_EXEC_PATH
export CROSS_COMPILE="riscv64-unknown-linux-musl"
export AR=${CROSS_COMPILE}-ar
export AS=${CROSS_COMPILE}-as
export LD=${CROSS_COMPILE}-ld
export RANLIB=${CROSS_COMPILE}-ranlib
export CC=${CROSS_COMPILE}-gcc
export CXX=${CROSS_COMPILE}-g++
export NM=${CROSS_COMPILE}-nm

function builddef() {
    #cd ${APP_DIR}
    ./configure \
    --prefix=$(pwd)/lib_x264 \
    --host=${CROSS_COMPILE} \
    --disable-asm \
    --enable-static \
    --extra-cflags="-fPIC -Wno-multichar -Wno-deprecated-declarations -Wno-unused-result -Wno-unused-variable -Wno-format -Wno-return-type -Wno-sign-compare -Wno-unused-label" \
    --extra-ldflags="-T /home/sunxiaopeng/src/work/k230/k230_sdk/src/big/mpp/userapps/sample/linker_scripts/riscv64/link.lds -lpthread -Wl,--whole-archive -Wl,--no-whole-archive -n --static"

    make clean
    if [ "$1" == "verbose" ]; then
        make V=1
    else
        make -j4
    fi
    make install
}
builddef $1

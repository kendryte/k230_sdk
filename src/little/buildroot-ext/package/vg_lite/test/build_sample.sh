if [ -z $VIVANTE_SDK_DIR ]; then
   echo
   echo ERROR: not found Vivante VGLite driver SDK directory
   echo You need export VIVANTE_SDK_DIR environment variable to specify driver SDK
   echo e.g
   echo     export VIVANTE_SDK_DIR=`pwd`/../DRV/build/sdk
   echo
   echo Make sure you can find VGLite driver library and header files.
   echo     \$VIVANTE_SDK_DIR/drivers/libvg_lite.so
   echo     \$VIVANTE_SDK_DIR/inc/vg_lite.h
   echo     \$VIVANTE_SDK_DIR/inc/vg_lite_error.h
   echo
   exit 0
fi

export SDK_DIR=$VIVANTE_SDK_DIR
make $@

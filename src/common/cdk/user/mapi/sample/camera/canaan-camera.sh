#!/bin/sh

YUV="360p"
MJPEG="720p"
H264="720p"
BULK=1

UDC=`ls /sys/class/udc`
CONFIGFS="/sys/kernel/config"
GADGET="$CONFIGFS/usb_gadget"
VID="0x29f1"
PID="0x0230"
SERIAL="0123456789"
MANUF=$(hostname)
PRODUCT="UVC Gadget"
USB_OTG="91540000.usb-otg"

if [ "$2" = "otg0" ]
then
    USB_OTG="91500000.usb-otg"
fi

if [ "$3" = "iso" ]
then
    BULK=0
fi

create_uvc() {
    CONFIG=$1
    FUNCTION=$2

    mkdir functions/$FUNCTION
    cd functions/$FUNCTION

    mkdir control/header/h/
    echo "0x0110" > control/header/h/bcdUVC
    echo "48000000" > control/header/h/dwClockFrequency
    ln -s control/header/h/ control/class/fs/
    ln -s control/header/h/ control/class/ss/

    echo $BULK > streaming_bulk
    echo 4 > uvc_num_request
    echo 3 > streaming_interval


    #YUV
    mkdir streaming/uncompressed/u/
    for i in $YUV
    do
        if [ $i = 360p ];then
        mkdir streaming/uncompressed/u/360p/
        echo "333333" > streaming/uncompressed/u/360p/dwFrameInterval
        echo "333333" > streaming/uncompressed/u/360p/dwDefaultFrameInterval
        echo "55296000" > streaming/uncompressed/u/360p/dwMaxBitRate
        echo "460800" > streaming/uncompressed/u/360p/dwMaxVideoFrameBufferSize
        echo "55296000" > streaming/uncompressed/u/360p/dwMinBitRate
        echo "360" > streaming/uncompressed/u/360p/wHeight
        echo "640" > streaming/uncompressed/u/360p/wWidth
        elif [ $i = 720p ];then
        mkdir streaming/uncompressed/u/720p/
        echo "333333" > streaming/uncompressed/u/720p/dwFrameInterval
        echo "333333" > streaming/uncompressed/u/720p/dwDefaultFrameInterval
        echo "29491200" > streaming/uncompressed/u/720p/dwMaxBitRate
        echo "1843200" > streaming/uncompressed/u/720p/dwMaxVideoFrameBufferSize
        echo "29491200" > streaming/uncompressed/u/720p/dwMinBitRate
        echo "720" > streaming/uncompressed/u/720p/wHeight
        echo "1280" > streaming/uncompressed/u/720p/wWidth
        elif [ $i = 1080p ];then
        mkdir streaming/uncompressed/u/1080p/
        echo "333333" > streaming/uncompressed/u/1080p/dwFrameInterval
        echo "333333" > streaming/uncompressed/u/1080p/dwDefaultFrameInterval
        echo "29491200" > streaming/uncompressed/u/1080p/dwMaxBitRate
        echo "4147200" > streaming/uncompressed/u/1080p/dwMaxVideoFrameBufferSize
        echo "29491200" > streaming/uncompressed/u/1080p/dwMinBitRate
        echo "1080" > streaming/uncompressed/u/1080p/wHeight
        echo "1920" > streaming/uncompressed/u/1080p/wWidth
        else
            echo "YUV $i is invalid!"
        fi
    done

    #MJPEG
    mkdir streaming/mjpeg/m/
    for i in $MJPEG
    do
        if [ $i = 360p ];then
        mkdir streaming/mjpeg/m/360p/
        echo "333333" > streaming/mjpeg/m/360p/dwFrameInterval
        echo "333333" > streaming/mjpeg/m/360p/dwDefaultFrameInterval
        echo "10240000" > streaming/mjpeg/m/360p/dwMaxBitRate
        echo "460800" > streaming/mjpeg/m/360p/dwMaxVideoFrameBufferSize
        echo "10240000" > streaming/mjpeg/m/360p/dwMinBitRate
        echo "360" > streaming/mjpeg/m/360p/wHeight
        echo "640" > streaming/mjpeg/m/360p/wWidth
        elif [ $i = 720p ];then
        mkdir streaming/mjpeg/m/720p/
        echo "333333" > streaming/mjpeg/m/720p/dwFrameInterval
        echo "333333" > streaming/mjpeg/m/720p/dwDefaultFrameInterval
        echo "20480000" > streaming/mjpeg/m/720p/dwMaxBitRate
        echo "1843200" > streaming/mjpeg/m/720p/dwMaxVideoFrameBufferSize
        echo "20480000" > streaming/mjpeg/m/720p/dwMinBitRate
        echo "720" > streaming/mjpeg/m/720p/wHeight
        echo "1280" > streaming/mjpeg/m/720p/wWidth
        elif [ $i = 1080p ];then
        mkdir streaming/mjpeg/m/1080p/
        echo "333333" > streaming/mjpeg/m/1080p/dwFrameInterval
        echo "333333" > streaming/mjpeg/m/1080p/dwDefaultFrameInterval
        echo "40960000" > streaming/mjpeg/m/1080p/dwMaxBitRate
        echo "4147200" > streaming/mjpeg/m/1080p/dwMaxVideoFrameBufferSize
        echo "40960000" > streaming/mjpeg/m/1080p/dwMinBitRate
        echo "1080" > streaming/mjpeg/m/1080p/wHeight
        echo "1920" > streaming/mjpeg/m/1080p/wWidth
        else
            echo "MJPEG $i is invalid!"
        fi
    done


    #FRAMEBASED
    mkdir streaming/framebased/fb/
    for i in $H264
    do
        if [ $i = 360p ];then
        mkdir streaming/framebased/fb/360p/
        echo "333333" > streaming/framebased/fb/360p/dwFrameInterval
        echo "333333" > streaming/framebased/fb/360p/dwDefaultFrameInterval
        echo "8192000" > streaming/framebased/fb/360p/dwMaxBitRate
        echo "8192000" > streaming/framebased/fb/360p/dwMinBitRate
        echo "360" > streaming/framebased/fb/360p/wHeight
        echo "640" > streaming/framebased/fb/360p/wWidth
        elif [ $i = 720p ];then
        mkdir streaming/framebased/fb/720p/
        echo "333333" > streaming/framebased/fb/720p/dwFrameInterval
        echo "333333" > streaming/framebased/fb/720p/dwDefaultFrameInterval
        echo "10240000" > streaming/framebased/fb/720p/dwMaxBitRate
        echo "10240000" > streaming/framebased/fb/720p/dwMinBitRate
        echo "720" > streaming/framebased/fb/720p/wHeight
        echo "1280" > streaming/framebased/fb/720p/wWidth
        elif [ $i = 1080p ];then
        mkdir streaming/framebased/fb/1080p/
        echo "333333" > streaming/framebased/fb/1080p/dwFrameInterval
        echo "333333" > streaming/framebased/fb/1080p/dwDefaultFrameInterval
        echo "15360000" > streaming/framebased/fb/1080p/dwMaxBitRate
        echo "15360000" > streaming/framebased/fb/1080p/dwMinBitRate
        echo "1080" > streaming/framebased/fb/1080p/wHeight
        echo "1920" > streaming/framebased/fb/1080p/wWidth
        else
            echo "H264 $i is invalid!"
        fi
    done

    mkdir streaming/header/h

    ln -s streaming/uncompressed/u/ streaming/header/h/
    ln -s streaming/mjpeg/m/ streaming/header/h/
    ln -s streaming/framebased/fb/ streaming/header/h/

    ln -s streaming/header/h/ streaming/class/fs
    ln -s streaming/header/h/ streaming/class/hs
    ln -s streaming/header/h/ streaming/class/ss

    cd -

    ln -s functions/$FUNCTION $CONFIG
}

delete_uvc() {
    CONFIG=$1
    FUNCTION=$2

    rm $CONFIG/$FUNCTION

    rm functions/$FUNCTION/control/class/*/h
    rm functions/$FUNCTION/streaming/class/*/h

    rm functions/$FUNCTION/streaming/header/h/u
    for i in $YUV
    do
        if [ $i = 360p ];then
        rmdir functions/$FUNCTION/streaming/uncompressed/u/360p
        elif [ $i = 720p ];then
        rmdir functions/$FUNCTION/streaming/uncompressed/u/720p
        elif [ $i = 1080p ];then
        rmdir functions/$FUNCTION/streaming/uncompressed/u/1080p
        else
            echo "YUV $i is invalid!"
        fi
    done
    rmdir functions/$FUNCTION/streaming/uncompressed/u

    rm functions/$FUNCTION/streaming/header/h/m
    for i in $MJPEG
    do
        if [ $i = 360p ];then
        rmdir functions/$FUNCTION/streaming/mjpeg/m/360p
        elif [ $i = 720p ];then
        rmdir functions/$FUNCTION/streaming/mjpeg/m/720p
        elif [ $i = 1080p ];then
        rmdir functions/$FUNCTION/streaming/mjpeg/m/1080p
        else
            echo "MJPEG $i is invalid!"
        fi
    done
    rmdir functions/$FUNCTION/streaming/mjpeg/m

    rm functions/$FUNCTION/streaming/header/h/fb
    for i in $H264
    do
        if [ $i = 360p ];then
        rmdir functions/$FUNCTION/streaming/framebased/fb/360p
        elif [ $i = 720p ];then
        rmdir functions/$FUNCTION/streaming/framebased/fb/720p
        elif [ $i = 1080p ];then
        rmdir functions/$FUNCTION/streaming/framebased/fb/1080p
        else
            echo "H264 $i is invalid!"
        fi
    done
    rmdir functions/$FUNCTION/streaming/framebased/fb

    rmdir functions/$FUNCTION/streaming/header/h
    rmdir functions/$FUNCTION/control/header/h
    rmdir functions/$FUNCTION
}

case "$1" in
    start)
        mount -t configfs none /sys/kernel/config/

        mkdir -p $GADGET/g1

        cd $GADGET/g1
        echo $VID > idVendor
        echo $PID > idProduct

        mkdir -p strings/0x409

        echo $SERIAL > strings/0x409/serialnumber
        echo $MANUF > strings/0x409/manufacturer
        echo $PRODUCT > strings/0x409/product

        echo "0x01" > bDeviceProtocol
        echo "0x02" > bDeviceSubClass
        echo "0xEF" > bDeviceClass

        mkdir configs/c.1
        echo 500 > configs/c.1/MaxPower
        echo 0xc0 > configs/c.1/bmAttributes
        mkdir configs/c.1/strings/0x409
        echo "Config 1" > configs/c.1/strings/0x409/configuration

        create_uvc configs/c.1 uvc.0

        echo $USB_OTG > UDC
        ;;
    stop)
        cd $GADGET/g1
        echo "" > UDC
        delete_uvc configs/c.1 uvc.0
        rmdir strings/0x409
        rmdir configs/c.1/strings/0x409
        rmdir configs/c.1

        cd $GADGET
        rmdir g1

        cd /
        umount /sys/kernel/config/
        ;;
    *)
        echo "Usage : $0 {start|stop} {otg0|otg1} [iso]"
esac

#!/bin/sh

mount -t configfs none /sys/kernel/config
cd /sys/kernel/config/usb_gadget

mkdir g2
cd g2

echo "0x29f1" > idVendor
echo "0x1230" > idProduct

mkdir configs/c.1
mkdir configs/c.2
mkdir functions/Loopback.0
mkdir functions/SourceSink.0

echo 0 > /sys/kernel/config/usb_gadget/g2/functions/SourceSink.0/pattern
# echo 524288 > /sys/kernel/config/usb_gadget/g2/functions/SourceSink.0/bulk_buflen
# echo 4 > /sys/kernel/config/usb_gadget/g2/functions/SourceSink.0/bulk_qlen
echo 2 > /sys/kernel/config/usb_gadget/g2/functions/SourceSink.0/isoc_interval
echo 8 > /sys/kernel/config/usb_gadget/g2/functions/SourceSink.0/isoc_maxpacket

echo 102400 > /sys/kernel/config/usb_gadget/g2/functions/Loopback.0/bulk_buflen

mkdir strings/0x409
mkdir configs/c.1/strings/0x409
mkdir configs/c.2/strings/0x409

echo "Canaan Inc." > strings/0x409/manufacturer
echo "USBtest Gadget" > strings/0x409/product
echo 20230618 > strings/0x409/serialnumber

echo "Conf 1" > configs/c.1/strings/0x409/configuration
echo "Conf 2" > configs/c.2/strings/0x409/configuration
echo 120 > configs/c.1/MaxPower

# SourceSink：驱动 set configuration 会选取 第一个 configuration
ln -s functions/Loopback.0 configs/c.2
ln -s functions/SourceSink.0 configs/c.1

echo 91500000.usb-otg > UDC
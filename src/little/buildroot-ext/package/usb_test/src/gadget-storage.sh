#!/bin/sh

cd /tmp/

mkdir /etc/configfs -p
export CONFIGFS_HOME=/etc/configfs
mount none $CONFIGFS_HOME -t configfs
 
mkdir $CONFIGFS_HOME/usb_gadget/g2
cd $CONFIGFS_HOME/usb_gadget/g2
 
echo 0x29f1 > idVendor
echo 0x4230 > idProduct
 
mkdir strings/0x409
echo "Canaan Inc." > strings/0x409/manufacturer
echo "StorageMem Gadget" > strings/0x409/product
echo 20230618 > strings/0x409/serialnumber
 
mkdir configs/c.1
 
mkdir configs/c.1/strings/0x409
echo mass_storage > configs/c.1/strings/0x409/configuration
 
mkdir functions/mass_storage.usb0
if `grep mmcblk0p3 /proc/cmdline >/dev/null` ; then
    echo /dev/mmcblk0p4 >functions/mass_storage.usb0/lun.0/file
elif `grep mmcblk1p3 /proc/cmdline >/dev/null` ; then
    echo /dev/mmcblk1p4 >functions/mass_storage.usb0/lun.0/file
fi

 
ln -s functions/mass_storage.usb0 configs/c.1
 
echo 91500000.usb-otg > UDC
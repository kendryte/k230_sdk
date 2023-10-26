#!/bin/sh

cd /tmp/
dd if=/dev/zero of=vfat.img bs=1M count=2
losetup /dev/loop0 vfat.img
mkfs.vfat /dev/loop0
mkdir vfat_mount_point -p
mount /dev/loop0 vfat_mount_point

mkdir /etc/configfs -p
export CONFIGFS_HOME=/etc/configfs
mount none $CONFIGFS_HOME -t configfs
 
mkdir $CONFIGFS_HOME/usb_gadget/g2
cd $CONFIGFS_HOME/usb_gadget/g2
 
echo 0x29f1 > idVendor
echo 0x3230 > idProduct
 
mkdir strings/0x409
echo "Canaan Inc." > strings/0x409/manufacturer
echo "Storage Gadget" > strings/0x409/product
echo 20230618 > strings/0x409/serialnumber
 
mkdir configs/c.1
 
mkdir configs/c.1/strings/0x409
echo mass_storage > configs/c.1/strings/0x409/configuration
 
mkdir functions/mass_storage.usb0
echo /dev/loop0 >functions/mass_storage.usb0/lun.0/file
 
ln -s functions/mass_storage.usb0 configs/c.1
 
echo 91500000.usb-otg > UDC
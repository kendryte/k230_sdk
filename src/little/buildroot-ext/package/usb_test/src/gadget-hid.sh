#!/bin/sh

################### TEST #################
# ./hid_gadget_test /dev/hidg0 keyboard
# Enter:--left-meta e
#
# ./hid_gadget_test /dev/hidg0 keyboard
# Enter:X Y [button] 
#    button:--b1/--b2/--b3
##########################################

mount -t configfs none /sys/kernel/config/
cd /sys/kernel/config/usb_gadget/
mkdir my_keyboard
cd my_keyboard

echo "0x01" > bDeviceProtocol
echo "0x02" > bDeviceSubClass
echo "0xEF" > bDeviceClass
echo 0x0419 > bcdDevice
echo 0x0200 > bcdUSB
echo "0x29f1" > idVendor
echo "0x2230" > idProduct
mkdir strings/0x409
echo "Canaan Inc." > strings/0x409/manufacturer
echo "HID Gadget" > strings/0x409/product
echo 20230618 > strings/0x409/serialnumber

mkdir functions/hid.usb0
mkdir functions/hid.usb1

#mouse
cd functions/hid.usb0
echo 1 > protocol
echo 4 > report_length
echo 1 > subclass
echo -ne \\x5\\x1\\x9\\x2\\xa1\\x1\\x9\\x1\\xa1\\x0\\x5\\x9\\x19\\x1\\x29\\x3\\x15\\x0\\x25\\x1\\x95\\x3\\x75\\x1\\x81\\x2\\x95\\x1\\x75\\x5\\x81\\x3\\x5\\x1\\x9\\x30\\x9\\x31\\x9\\x38\\x15\\x81\\x25\\x7f\\x75\\x8\\x95\\x3\\x81\\x6\\xc0\\xc0 > report_desc
cd ../../

#keyboard
cd functions/hid.usb1
echo 2 > protocol
echo 8 > report_length
echo 1 > subclass
echo -ne \\x5\\x1\\x9\\x6\\xa1\\x1\\x5\\x7\\x19\\xe0\\x29\\xe7\\x15\\x0\\x25\\x1\\x75\\x1\\x95\\x8\\x81\\x2\\x95\\x1\\x75\\x8\\x81\\x3\\x95\\x5\\x75\\x1\\x5\\x8\\x19\\x1\\x29\\x5\\x91\\x2\\x95\\x1\\x75\\x3\\x91\\x3\\x95\\x6\\x75\\x8\\x15\\x0\\x25\\xFF\\x5\\x7\\x19\\x0\\x29\\x65\\x81\\x0\\xc0 > report_desc
cd ../../

mkdir configs/c.1
 
mkdir configs/c.1/strings/0x409
echo hid > configs/c.1/strings/0x409/configuration

ln -s functions/hid.usb0/ configs/c.1/
ln -s functions/hid.usb1/ configs/c.1/

echo 91540000.usb-otg > UDC


rtl8188fu for linux
===================


**PSA** public service announcement  
-------------------------------------
This driver will only work with kernels <= v5.10  
.. there no use for a new kernel  
.. sorry for that  

the purpose of this repository is to rip/strip unneeded parts from the vendor driver  
because of ..  

rtl8188fu linux driver for wireless bgn device

end of PSA
----------

Note:
This is an USB2 only adapter,  
Source is ripped out of firefly source tree found at  
https://gitlab.com/TeeFirefly/linux-kernel 

<u>If one USB-ID is missing, please mail me.</u>  

build/load/function tested with v4.19  

Building and install driver
---------------------------

for building type  
`make`  

for load the driver  
`sudo modprobe cfg80211`  
`sudo insmod rtl8188fu.ko`  

You need to install the needed fw with  
`sudo make installfw`  

If you need to crosscompile use  
`ARCH= CROSS_COMPILE= KSRC=`  
while calling `make` i.e.  

`make ARCH="arm" CROSS_COMPILE=armv5tel-softfloat-linux-gnueabi- KSRC=/home/linux-master modules`  

Please use prefix **rtl8188fu** if you want to mail me  
But please please don't, I have enough to do.  
TIA  

Hans Ulli Kroll <linux@ulli-kroll.de>

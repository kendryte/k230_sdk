#!/bin/bash

find . | cpio -o -H newc > ../rootfs-final.cpio
gzip -f ../rootfs-final.cpio

#! /bin/sh

is_emmc=false
is_sd=false
is_flash=false
data_dir=/tmp

###### step 1 ######
###### 解压升级包 ######
if [ $1 -eq 0 ]; then
    rm $data_dir/ota_package.kpk
    echo 3 > /proc/sys/vm/drop_caches

    sed -n -e "1,/^# END OF THE SCRIPT/p" "$0" > "$data_dir/script.bin" 2>/dev/null
    script_len=`ls -l $data_dir/script.bin | awk '{print $5}'`
    dd if="$0" of="$data_dir/ota_package.zip" skip=$script_len iflag=skip_bytes
    if [ $? -ne 0 ]; then
        echo "dd ota_package.zip fail"
        exit 1
    fi
    echo "dd ota_package.zip success"

    rm $data_dir/ota_package -rf
elif [ $1 -eq 1 ]; then
    cd $data_dir
    rm $data_dir/ota_package.bin -rf
    unzip $data_dir/ota_package.zip
    if [ $? -ne 0 ]; then
        echo "decompress ota_package.zip fail"
        exit 1
    fi
    echo "decompress ota_package.zip success"
    rm $data_dir/ota_package.zip -rf

    echo 3 > /proc/sys/vm/drop_caches

    cd $data_dir/ota_package
    ###### step 2 ######
    ###### 判断当前启动盘类型######
    cat /proc/cmdline | grep mmcblk0p3
    if [ $? -eq 0 ]; then
        is_emmc=true
    fi
    cat /proc/cmdline | grep mmcblk1p3
    if [ $? -eq 0 ]; then
        is_sd=true
    fi
    cat /proc/cmdline | grep ubifs
    if [ $? -eq 0 ]; then
        is_flash=true
    fi

    ###### step 3 ######
    ###### 升级flash分区 ######
    if [ "$is_flash" == true ]; then
        if [ -f u-boot-spl-k230-swap.bin ]; then
            echo "u-boot-spl-k230-swap.bin"
            flash_erase /dev/mtd0 0 0
            dd if=u-boot-spl-k230-swap.bin of=/dev/mtd0
        fi
        if [ -f u-boot.img ]; then
            echo "u-boot.img"
            flash_erase /dev/mtd1 0 0
            dd if=u-boot.img of=/dev/mtd1
        fi
        if [ -f fh_quick_boot.bin ]; then
            echo "fh_quick_boot.bin"
            flash_erase /dev/mtd2 0 0
            dd if=fh_quick_boot.bin of=/dev/mtd2
        fi
        if [ -f fh_face_data.bin ]; then
            echo "fh_face_data.bin"
            flash_erase /dev/mtd3 0 0
            dd if=fh_face_data.bin of=/dev/mtd3
        fi
        if [ -f fh_sensor_cfg.bin ]; then
            echo "fh_sensor_cfg.bin"
            flash_erase /dev/mtd4 0 0
            dd if=fh_sensor_cfg.bin of=/dev/mtd4
        fi
        if [ -f fh_speckle.bin ]; then
            echo "fh_speckle.bin"
            flash_erase /dev/mtd5 0 0
            dd if=fh_speckle.bin of=/dev/mtd5
        fi
        if [ -f rtt_system.bin ]; then
            echo "rtt_system.bin"
            flash_erase /dev/mtd6 0 0
            dd if=rtt_system.bin of=/dev/mtd6
        fi
        if [ -f fh_fastboot_app.elf ]; then
            echo "fh_fastboot_app.elf"
            flash_erase /dev/mtd7 0 0
            dd if=fh_fastboot_app.elf of=/dev/mtd7
        fi
        if [ -f linux_system.bin ]; then
            echo "linux_system.bin"
            flash_erase /dev/mtd8 0 0
            dd if=linux_system.bin of=/dev/mtd8
        fi
    fi
    ###### step 4 ######
    ###### 升级mmc分区 ######
    if [ "$is_emmc" == true ]; then
        if [ -f rtt_system.bin ]; then
            echo "rtt_system.bin"
            dd if=rtt_system.bin of=/dev/mmcblk0p1
        fi
        if [ -f linux_system.bin ]; then
            echo "linux_system.bin"
            dd if=linux_system.bin of=/dev/mmcblk0p2
        fi
        if [ -d rootfs/sharefs ]; then
            if [ "`ls -A rootfs/sharefs`" != "" ]; then
                cp -dr rootfs/sharefs/* /sharefs/
                if [ $? -ne 0 ]; then
                    echo "copy fatfs files fail"
                    exit 1
                fi
                echo "copy fatfs files success"
            fi
        fi
    fi
    if [ "$is_sd" == true ]; then
        if [ -f rtt_system.bin ]; then
            echo "rtt_system.bin"
            dd if=rtt_system.bin of=/dev/mmcblk1p1
        fi
        if [ -f linux_system.bin ]; then
            echo "linux_system.bin"
            dd if=linux_system.bin of=/dev/mmcblk1p2
        fi
        if [ -d rootfs/sharefs ]; then
            if [ "`ls -A rootfs/sharefs`" != "" ]; then
                cp -dr rootfs/sharefs/* /sharefs/
                if [ $? -ne 0 ]; then
                    echo "copy fatfs files fail"
                    exit 1
                fi
                echo "copy fatfs files success"
            fi
        fi
    fi
    ###### step 5 ######
    ###### 文件系统文件 ######
    if [ -d rootfs ]; then
        if [ "`ls -A rootfs`" != "" ]; then
            cp -dr rootfs/* /
            if [ $? -ne 0 ]; then
                echo "copy system files fail"
                exit 1
            fi
            echo "copy system files success"
        fi
    fi
    sync
    echo "OTA update success"
fi
exit 0
# END OF THE SCRIPT

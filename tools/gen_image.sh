#!/bin/bash
source ${K230_SDK_ROOT}/.config

set -e;
#apt-get install python3-pip
#pip3  install pycryptodome
#pip3 install gmssl

mkimage="${UBOOT_BUILD_DIR}/tools/mkimage"
mkenvimage="${UBOOT_BUILD_DIR}/tools/mkenvimage"
firmware_gen="${K230_SDK_ROOT}/tools/firmware_gen.py"
genimage="${BUILDROOT_BUILD_DIR}/host/bin/genimage"
mkfs="${BUILDROOT_BUILD_DIR}/host/sbin/mkfs.ext4"
k230_gzip="${K230_SDK_ROOT}/tools/k230_priv_gzip -n8 "
GENIMAGE_CFG_DIR="${K230_SDK_ROOT}/tools/gen_image_cfg"
GENIMAGE_CFG_SD="${GENIMAGE_CFG_DIR}/genimage-sdcard.cfg"
GENIMAGE_CFG_SD_AES="${GENIMAGE_CFG_DIR}/genimage-sdcard_aes.cfg"
GENIMAGE_CFG_SD_SM="${GENIMAGE_CFG_DIR}/genimage-sdcard_sm.cfg"
#GENIMAGE_CFG_SD_DDR4="${GENIMAGE_CFG_DIR}/genimage-sdcard_ddr4.cfg"
GENIMAGE_CFG_SPI_NOR="${GENIMAGE_CFG_DIR}/genimage-spinor.cfg"
GENIMAGE_CFG_SPI_NAND="${GENIMAGE_CFG_DIR}/genimage-spinand.cfg"
GENIMAGE_CFG_SD_REMOTE="${GENIMAGE_CFG_DIR}/genimage-sdcard_remote.cfg"


quick_boot_cfg_data_file="${GENIMAGE_CFG_DIR}/data/quick_boot.bin"
face_database_data_file="${GENIMAGE_CFG_DIR}/data/face_data.bin"
sensor_cfg_data_file="${GENIMAGE_CFG_DIR}/data/sensor_cfg.bin"
ai_mode_data_file="${BUILD_DIR}/images/big-core/ai_mode.bin" #"${GENIMAGE_CFG_DIR}/data/ai_mode.bin"
speckle_data_file="${GENIMAGE_CFG_DIR}/data/speckle.bin"
rtapp_data_file="${BUILD_DIR}/images/big-core/fastboot_app.elf"


#生成版本号
gen_version()
{
	local ver_file="etc/version/release_version"
	cd  "${BUILD_DIR}/images/little-core/rootfs" ; 
	mkdir -p etc/version/


	set +e; commitid=$(awk -F- '/^[^#]/ { print $6}' ${K230_SDK_ROOT}/tools/post_copy_rootfs/${ver_file});set -e;
	set +e; last_tag=$(awk -F- '/^[^#]/ { print $1}' ${K230_SDK_ROOT}/tools/post_copy_rootfs/${ver_file}) ;set -e;
	

	[ "${commitid}" != "" ] || commitid="unkonwn"
	[ "${last_tag}" != "" ] || last_tag="unkonwn"

	git rev-parse --short HEAD  &&  commitid=$(git rev-parse --short HEAD) 
	git describe --tags `git rev-list --tags --max-count=1` && last_tag=$(git describe --tags `git rev-list --tags --max-count=1`)

	ver="${last_tag}-$(date "+%Y%m%d-%H%M%S")-$(whoami)-$(hostname)-${commitid}"
	echo -e "#############SDK VERSION######################################" >${ver_file}
	echo -e ${ver} >> ${ver_file}
	echo -e "##############################################################" >>${ver_file}
	echo "build version: ${ver}"

	mkdir -p ${K230_SDK_ROOT}/tools/post_copy_rootfs/etc/version/
	cp -f ${ver_file}  ${K230_SDK_ROOT}/tools/post_copy_rootfs/${ver_file}

	cd -;

}




#生成可用uboot引导的linux版本文件
gen_linux_bin ()
{
	cd  "${BUILD_DIR}/images/little-core/" ; 

	ROOTFS_BASE=`cat hw/k230.dts.txt | grep initrd-start | awk -F " " '{print $4}' | awk -F ">" '{print $1}'`
	ROOTFS_SIZE=`ls -lt rootfs-final.cpio.gz | awk '{print $5}'`
	((ROOTFS_END= $ROOTFS_BASE + $ROOTFS_SIZE))
	ROOTFS_END=`printf "0x%x" $ROOTFS_END`
	sed -i "s/linux,initrd-end = <0x0 .*/linux,initrd-end = <0x0 $ROOTFS_END>;/g" hw/k230.dts.txt

	${LINUX_BUILD_DIR}/scripts/dtc/dtc -I dts -O dtb hw/k230.dts.txt  >k230.dtb;		
	${k230_gzip}  -f -k fw_payload.bin; 
	sed -i -e "1s/\x08/\x09/"  fw_payload.bin.gz
	echo a>rd;
	${mkimage} -A riscv -O linux -T multi -C gzip -a ${CONFIG_MEM_LINUX_SYS_BASE} -e ${CONFIG_MEM_LINUX_SYS_BASE} -n linux -d fw_payload.bin.gz:rd:k230.dtb  ulinux.bin;

	cp ulinux.bin tmp.bin ; python3 ${firmware_gen}  -i tmp.bin -o linux_system.bin -n ;  
	if [ "${CONFIG_GEN_SECURITY_IMG}" = "y" ] ; then 
		cp ulinux.bin tmp.bin ; python3 ${firmware_gen}  -i tmp.bin -o linux_system_aes.bin -a ;  
		cp ulinux.bin tmp.bin ; python3 ${firmware_gen}  -i tmp.bin -o linux_system_sm.bin -s ;  
	fi
	chmod a+rw linux_system.bin;
	rm -rf tmp.bin rd;
}

#生成ext2 格式镜像；
gen_final_ext2 ()
{
	cd  "${BUILD_DIR}/images/little-core/" ; 
	rm -rf rootfs.ext*
	rm -rf rootfs/dev/console
	rm -rf rootfs/dev/null

	fakeroot ${mkfs} -d rootfs  -r 1 -N 0 -m 1 -L "rootfs" -O ^64bit rootfs.ext4 80M
}


#生成可用uboot引导的rtt版本文件
gen_rtt_bin ()
{
	cd "${BUILD_DIR}/images/big-core/" ; 	
	cp big-opensbi/platform/kendryte/fpgac908/firmware/fw_payload.bin .;
	${k230_gzip}  -f -k fw_payload.bin; 
	sed -i -e "1s/\x08/\x09/"  fw_payload.bin.gz
	${mkimage} -A riscv -O opensbi -T multi -C gzip -a ${CONFIG_MEM_RTT_SYS_BASE} -e ${CONFIG_MEM_RTT_SYS_BASE} -n rtt  -d fw_payload.bin.gz  rtt.bin;
	cp rtt.bin tmp.bin; python3  ${firmware_gen}   -i tmp.bin -o rtt_system.bin -n;
	if [ "${CONFIG_GEN_SECURITY_IMG}" = "y" ] ; then 
		cp rtt.bin tmp.bin; python3  ${firmware_gen}   -i tmp.bin -o rtt_system_aes.bin -a;
		cp rtt.bin tmp.bin; python3  ${firmware_gen}   -i tmp.bin -o rtt_system_sm.bin -s;
	fi
	chmod a+rw rtt_system.bin
	rm -rf tmp.bin fw_payload.bin*;
}
#cfg_quick_boot,
#gen_part_bin quick_boot_cfg_data_file   -n/-a/-s  name 0x80000 
gen_cfg_part_bin()
{
	local file_full_path="$1"
	local filename=$(basename ${file_full_path})
	local arg="$2"
	local name="$3"
	local add="$4"

	cd  "${BUILD_DIR}/images/";
	mkdir -p ${GENIMAGE_CFG_DIR}/data
	[ -f ${file_full_path} ] || (echo ${filename} >${file_full_path} )
	cp ${file_full_path} .
	${k230_gzip} -f -k ${filename}  #gzip
	sed -i -e "1s/\x08/\x09/"  ${filename}.gz
	#add uboot head 
	${mkimage} -A riscv -O linux -T firmware -C gzip -a ${add} -e ${add} -n ${name}  -d ${filename}.gz  ${filename}.gzu;
	python3  ${firmware_gen}   -i ${filename}.gzu -o fh_${filename} ${arg}; #add k230 firmware head
	rm -rf ${filename}  ${filename}.gz ${filename}.gzu
}


#生成sd卡镜像文件
gen_image() 
{
	local cfg="$1" ; #"genimage-sdcard.cfg"
	local image_name="$2"; #"sysimage-sdcard.img"
	cd  "${BUILD_DIR}/images/";
	GENIMAGE_TMP="genimage.tmp" ;	rm -rf "${GENIMAGE_TMP}";
	${genimage}   	--rootpath "little-core/rootfs/"  --tmppath "${GENIMAGE_TMP}"    \
					--inputpath "$(pwd)"  	--outputpath "$(pwd)"	--config "${cfg}"

	rm -rf "${GENIMAGE_TMP}"  
	gzip -k -f ${image_name}
	chmod a+rw ${image_name} ${image_name}.gz;
}

gen_image_spinor_proc_ai_mode()
{
	cd ${BUILD_DIR}/images/big-core/; #cp  root/bin/test.kmodel   root/bin/test1.kmodel;
	rm -rf ai_mode ai_mode.bin; mkdir -p ai_mode; cp  root/bin/*.kmodel ai_mode;
	cd ai_mode;
	local all_kmode=""
	local size=""
	local start="0"

	for file in $(ls *.kmodel) ; do 
		truncate -s %128 ${file};
		cat ${file} >> ../ai_mode.bin
		all_kmode="${all_kmode} ${file}";
		size=$(du -sb ${file} | cut -f1 )
		echo "${file%%\.*}_start=\"${start}\"" >>file_size.txt;
		echo "${file%%\.*}_size=\"${size}\"" >>file_size.txt;
		start=$((${start}+${size}))		
	done
	echo "all_kmode=\"${all_kmode}\""  >>file_size.txt
	echo "all_size=\"${start}\""  >>file_size.txt
}
gen_image_spinor_proc_ai_mode_replace()
{

	local fstart="" #
	local fsize=""

	for f in ${all_kmode};
	do 
		eval fstart="\${${f%%\.*}_start}"
		eval fsize="\${${f%%\.*}_size}"
		fstart=$(printf "0x%x" $((${fstart} + ${CONFIG_MEM_AI_MODEL_BASE})))
		fsize=$(printf "0x%x" ${fsize})
		sed -i "s/_bin_$f,/(char*)${fstart},/g" ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c
		sed -i "s/sizeof(_bin_$f)/${fsize}/g" ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c
	done
}

gen_image_spinor()
{
	cd  ${BUILD_DIR}/;rm -rf images_bak;cp images images_bak -r;

	#裁剪小核rootfs
	cd ${BUILD_DIR}/images/little-core/rootfs/; 
	rm -rf lib/modules/;
	rm -rf lib/libstdc++*;
	rm -rf usr/lib/libcrypto.so*;
	rm -rf usr/bin/fio;
	rm -rf mnt/*;
	rm -rf app/;
	rm -rf lib/tuning-server;	
	rm -rf usr/bin/stress-ng  bin/bash usr/sbin/sshd usr/bin/trace-cmd;
	#find . -name *.ko | xargs rm -rf ;
	fakeroot -- ${K230_SDK_ROOT}/tools/mkcpio-rootfs.sh; 


	#裁剪大核romfs;
	#$(RT-SMART_SRC_PATH)/userapps/root	
	#backup ai mode and fastapp 
	cd ${BUILD_DIR}/images/big-core/;
	gen_image_spinor_proc_ai_mode;
	source ${BUILD_DIR}/images/big-core/ai_mode/file_size.txt
	

	#${K230_SDK_ROOT}/tools/genromfs -V ai -v -a 32 -f ai_mode.bin -d binbak/ 

	#rtapp and ai mode
	cd ${BUILD_DIR}/images/big-core/root/bin/;fasb_app_size=$(du -sb fastboot_app.elf | cut -f1);cp  fastboot_app.elf ${BUILD_DIR}/images/big-core/;
	find . -type f  -not -name init.sh   | xargs rm -rf ; 
	echo a>fastboot_app.elf ; for file in ${all_kmode} ;do echo k>${file} ;done
	cd ${RTSMART_SRC_DIR}/userapps/; python3 ../tools/mkromfs.py ${BUILD_DIR}/images/big-core/root/  ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c;
	
	sed -i "s/_bin_fastboot_app_elf,/(char*)${CONFIG_MEM_RTAPP_BASE},/g" ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c
	sed -i "s/sizeof(_bin_fastboot_app_elf)/${fasb_app_size}/g" ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c
	gen_image_spinor_proc_ai_mode_replace
	


	#rtapp and ai_mode mount
	# local tmpl=$(grep _root_dirent ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c  -rn | head -n1 | cut -d: -f1)
	# #tmpl=$((${tmpl}+1))
	# sed -i "/automodify/d" ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c
	# sed -i "${tmpl}a {ROMFS_DIRENT_DIR, \"ai_mode\", RT_NULL, 0},//automodify" ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c

	# sed -i "/automodify/d" ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/mnt.c
	# sed -i "43a rt_kprintf(\"wjxwjx\"); if(dfs_mount(RT_NULL, \"/ai_mode\", \"rom\", 0, ${CONFIG_MEM_AI_MODEL_BASE}))rt_kprintf(\"mount ai_mode error\\\\n\");//automodify" ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/mnt.c


	cd ${K230_SDK_ROOT};make rt-smart-kernel big-core-opensbi;
	cd "${BUILD_DIR}/images/big-core/"
	cp ${BUILD_DIR}/big/rt-smart/rtthread.* .;
	cp ${BUILD_DIR}/common/big-opensbi/platform/kendryte/fpgac908/firmware/fw_payload.bin .;
	${k230_gzip}  -f -k fw_payload.bin; 
	sed -i -e "1s/\x08/\x09/"  fw_payload.bin.gz
	${mkimage} -A riscv -O opensbi -T multi -C gzip -a ${CONFIG_MEM_RTT_SYS_BASE} -e ${CONFIG_MEM_RTT_SYS_BASE} -n rtt  -d fw_payload.bin.gz  rtt.bin;
	cp rtt.bin tmp.bin; python3  ${firmware_gen}   -i tmp.bin -o rtt_system.bin -n;

	#gen_part_bin quick_boot_cfg_data_file   -n/-a/-s  name 0x80000 
	gen_cfg_part_bin ${quick_boot_cfg_data_file} -n  quick_boot_cfg  ${CONFIG_MEM_QUICK_BOOT_CFG_BASE}
	gen_cfg_part_bin ${face_database_data_file} -n  face_db  ${CONFIG_MEM_FACE_DATA_BASE}
	gen_cfg_part_bin ${sensor_cfg_data_file} -n  sensor_cfg  ${CONFIG_MEM_SENSOR_CFG_BASE}
	gen_cfg_part_bin ${ai_mode_data_file} -n  ai_mode ${CONFIG_MEM_AI_MODEL_BASE}
	gen_cfg_part_bin ${speckle_data_file} -n speckle ${CONFIG_MEM_SPECKLE_BASE}
	gen_cfg_part_bin ${rtapp_data_file} -n  rtapp  ${CONFIG_MEM_RTAPP_BASE}


	#生成spinor 镜像；
	gen_image ${GENIMAGE_CFG_SPI_NOR} sysimage-spinor32m.img

	#恢复删除前状态；
	cd  ${BUILD_DIR}/;rm -rf images_spinor;mv  images images_spinor; mv  images_bak images; 
	cp images_spinor/sysimage-spinor32m*.img images;
	return;
}
gen_image_spinand()
{
	cd  ${BUILD_DIR}/;rm -rf images_bak;cp images images_bak -r;

	#裁剪小核rootfs
	cd ${BUILD_DIR}/images/little-core/rootfs/; 
	rm -rf lib/modules/;
	rm -rf lib/libstdc++*;
	rm -rf usr/lib/libcrypto.so*;
	rm -rf usr/bin/fio;
	rm -rf mnt/*;
	rm -rf app/;
	rm -rf lib/tuning-server;	
	rm -rf usr/bin/stress-ng  bin/bash usr/sbin/sshd usr/bin/trace-cmd;
	#find . -name *.ko | xargs rm -rf ;
	fakeroot -- ${K230_SDK_ROOT}/tools/mkcpio-rootfs.sh; 


	#裁剪大核romfs;
	#$(RT-SMART_SRC_PATH)/userapps/root
	cd ${BUILD_DIR}/images/big-core/root/bin/; rm -rf sample_vicap_dump.elf sample_sys_init.elf  sample_venc.elf sample_dw200.elf sample_vdss.elf sample_vdd_r.elf sample_dpu* sample_dma* ./dpu;
	cd ${RTSMART_SRC_DIR}/userapps/; python3 ../tools/mkromfs.py ${BUILD_DIR}/images/big-core/root/  ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c;
	cd ${K230_SDK_ROOT};make rt-smart-kernel big-core-opensbi;
	cd "${BUILD_DIR}/images/big-core/"
	cp ${BUILD_DIR}/big/rt-smart/rtthread.* .;
	cp ${BUILD_DIR}/common/big-opensbi/platform/kendryte/fpgac908/firmware/fw_payload.bin .;
	${k230_gzip}  -f -k fw_payload.bin; 
	${mkimage} -A riscv -O opensbi -T multi -C gzip -a ${CONFIG_MEM_RTT_SYS_BASE} -e ${CONFIG_MEM_RTT_SYS_BASE} -n rtt  -d fw_payload.bin.gz  rtt.bin;
	cp rtt.bin tmp.bin; python3  ${firmware_gen}   -i tmp.bin -o rtt_system.bin -n;
	
	#生成spinor 镜像；
	gen_image ${GENIMAGE_CFG_SPI_NAND} sysimage-spinand32m.img

	#恢复删除前状态；
	cd  ${BUILD_DIR}/;rm -rf images_spinand;mv  images images_spinand; mv  images_bak images; 
	cp images_spinand/sysimage-spinand32m.img* images;
	return;
}

gen_env_bin()
{
	cd  "${BUILD_DIR}/images/";

	sed -i -e "/^quick_boot/d"  ${GENIMAGE_CFG_SPI_NOR}.jffs2.env
	sed -i -e "/quick_boot/d"  ${GENIMAGE_CFG_SPI_NAND}.env
	sed -i -e "/^quick_boot/d"  ${GENIMAGE_CFG_SD}.env
	sed -i -e "/restore_img/d"  ${GENIMAGE_CFG_SD}.env

	if [ "${CONFIG_QUICK_BOOT}" != "y" ] || [ "${CONFIG_REMOTE_TEST_PLATFORM}" = "y" ] ; then 
		echo "quick_boot=false" >> ${GENIMAGE_CFG_SPI_NOR}.jffs2.env
		echo "quick_boot=false" >> ${GENIMAGE_CFG_SPI_NAND}.env
		echo "quick_boot=false" >> ${GENIMAGE_CFG_SD}.env
	fi
	if [ "${CONFIG_REMOTE_TEST_PLATFORM}" = "y" ] ; then 
		echo "restore_img=mmc dev 0; mmc read 0x10000 0x200000 0x40000; gzwrite mmc 1 0x10000 0x8000000; reset" >> ${GENIMAGE_CFG_SD}.env
	fi

	${mkenvimage} -s 0x10000 -o jffs2.env ${GENIMAGE_CFG_SPI_NOR}.jffs2.env
	${mkenvimage} -s 0x10000 -o ubi.env ${GENIMAGE_CFG_SPI_NAND}.env
	${mkenvimage} -s 0x10000 -o env.env  ${GENIMAGE_CFG_SD}.env
}
gen_version;
gen_linux_bin;
gen_final_ext2;
gen_rtt_bin;

gen_env_bin;

mkdir -p ${BUILD_DIR}/images/big-core/app
cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/*   ${BUILD_DIR}/images/big-core/app
cp ${K230_SDK_ROOT}/src/common/cdk/user/out/big/*  ${BUILD_DIR}/images/big-core/app

if [ "${CONFIG_REMOTE_TEST_PLATFORM}" = "y" ] ; then 
	${K230_SDK_ROOT}/tools/remote_test_platform.sh
	gen_image ${GENIMAGE_CFG_SD_REMOTE}   sysimage-sdcard.img
else
	gen_image ${GENIMAGE_CFG_SD}   sysimage-sdcard.img
fi

if [ "${CONFIG_GEN_SECURITY_IMG}" = "y" ] ; then 
	gen_image  ${GENIMAGE_CFG_SD_AES}  sysimage-sdcard_aes.img
	gen_image ${GENIMAGE_CFG_SD_SM}  sysimage-sdcard_sm.img
fi

gen_image_spinor 
gen_image_spinand; 



cd  ${BUILD_DIR}/images/
rm -rf  sysimage-sdcard_aes.img  sysimage-sdcard_sm.img  *.vfat










#!/bin/bash
source ${K230_SDK_ROOT}/.config
source ${K230_SDK_ROOT}/.last_conf


#依赖 BUILD_DIR，K230_SDK_ROOT
copye_file_to_images()
{
	mkdir -p ${BUILD_DIR}/images/little-core/rootfs;
	mkdir -p ${BUILD_DIR}/images/big-core/app;

	if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ]; then
		cd ${K230_SDK_ROOT};
		mkdir -p ${BUILD_DIR}/images/big-core/app;
		cp -rf src/big/rt-smart/userapps/root ${BUILD_DIR}/images/big-core/

		cp -f ${RTT_SDK_BUILD_DIR}/rtthread.* ${BUILD_DIR}/images/big-core/
		cp -rf ${BIG_OPENSBI_BUILD_DIR}/ ${BUILD_DIR}/images/big-core/
		if [ "${CONFIG_SUPPORT_LINUX}" != "y" ]; then
			echo a> ${BUILD_DIR}/images/little-core/linux_system.bin;
			echo a> ${BUILD_DIR}/images/little-core/rootfs.ext4;
		fi
	fi

	if [ "${CONFIG_SUPPORT_LINUX}" = "y" ]; then
		cp -rf ${BUILDROOT_BUILD_DIR}/images/* ${BUILD_DIR}/images/little-core/
		cp -rf ${LINUX_BUILD_DIR}/arch/riscv/boot/Image ${BUILD_DIR}/images/little-core/
		cp -rf ${LITTLE_OPENSBI_BUILD_DIR}/ ${BUILD_DIR}/images/little-core/
		cp -f ${LITTLE_OPENSBI_BUILD_DIR}/platform/generic/firmware/fw_*.bin ${BUILD_DIR}/images/little-core/
		cp -f ${LITTLE_OPENSBI_BUILD_DIR}/platform/generic/firmware/fw_*.elf ${BUILD_DIR}/images/little-core/

		mkdir -p ${BUILD_DIR}/images/little-core/ko-apps
		cp -rf ${LINUX_BUILD_DIR}/rootfs/* ${BUILD_DIR}/images/little-core/ko-apps/

		rm ${BUILD_DIR}/images/little-core/rootfs -rf
		mkdir -p ${BUILD_DIR}/images/little-core/rootfs
		cd ${BUILD_DIR}/images/little-core/rootfs; 
		fakeroot -- cpio -idm < ../rootfs.cpio ; 

		cp -rf ${K230_SDK_ROOT}/tools/ota/ota_public.pem  	${BUILD_DIR}/images/little-core/rootfs/etc/
		cp -rf ${K230_SDK_ROOT}/board/common/post_copy_rootfs/*  	${BUILD_DIR}/images/little-core/rootfs/
		fakeroot -- cp -rf ${BUILD_DIR}/images/little-core/ko-apps/* ${BUILD_DIR}/images/little-core/rootfs/
		cd ${BUILD_DIR}/images/little-core/rootfs/; \
		fakeroot -- ${K230_SDK_ROOT}/tools/mkcpio-rootfs.sh; \
		cd ../;  tar -zcf rootfs-final.tar.gz rootfs;

		if [ "${CONFIG_SUPPORT_RTSMART}" != "y" ]; then
			echo a> ${BUILD_DIR}/images/big-core/rtt_system.bin;
			echo a> ${BUILD_DIR}/images/big-core/rtt_system_aes.bin;
			echo a> ${BUILD_DIR}/images/big-core/rtt_system_sm.bin;
		fi

	fi
}

#生成版本号
gen_version()
{
	local ver_file="etc/version/release_version"
	local post_copy_rootfs_dir="${K230_SDK_ROOT}/board/common/post_copy_rootfs"
	cd  "${BUILD_DIR}/images/little-core/rootfs" ; 
	mkdir -p etc/version/


	set +e; commitid=$(awk -F- '/^[^#]/ { print $6}' ${post_copy_rootfs_dir}/${ver_file});set -e;
	set +e; last_tag=$(awk -F- '/^[^#]/ { print $1}' ${post_copy_rootfs_dir}/${ver_file}) ;set -e;
	

	[ "${commitid}" != "" ] || commitid="unkonwn"
	[ "${last_tag}" != "" ] || last_tag="unkonwn"

	git rev-parse --short HEAD  &&  commitid=$(git rev-parse --short HEAD) 
	git describe --tags `git rev-list --tags --max-count=1` && last_tag=$(git describe --tags `git rev-list --tags --max-count=1`)

	ver="${last_tag}-$(date "+%Y%m%d-%H%M%S")-$(whoami)-$(hostname)-${commitid}"
	echo -e "#############SDK VERSION######################################" >${ver_file}
	echo -e ${ver} >> ${ver_file}
	echo -e "##############################################################" >>${ver_file}
	echo "build version: ${ver}"

	mkdir -p ${post_copy_rootfs_dir}/etc/version/
	cp -f ${ver_file}  ${post_copy_rootfs_dir}/${ver_file}

	cd -;
}
#add_firmHead  xxx.bin  "-n"
#output fn_$1 fa_$1 fs_$1
add_firmHead()
{
	local filename="$1"
	local firmware_gen="${K230_SDK_ROOT}/tools/firmware_gen.py"
	if [ $# -ge 2 ]; then 
		firmArgs="$2" #add k230 firmware head
		cp ${filename} ${filename}.t; 					python3  ${firmware_gen}   -i ${filename}.t -o f${firmArgs##-}${filename} ${firmArgs};
	else 
		 #add k230 firmware head
		firmArgs="-n"; cp ${filename} ${filename}.t;  	python3  ${firmware_gen}   -i ${filename}.t -o f${firmArgs##-}_${filename} ${firmArgs};

		if [ "${CONFIG_GEN_SECURITY_IMG}" = "y" ];then
			firmArgs="-s";cp ${filename} ${filename}.t;	python3  ${firmware_gen}   -i ${filename}.t -o f${firmArgs##-}_${filename} ${firmArgs};
			firmArgs="-a";cp ${filename} ${filename}.t;	python3  ${firmware_gen}   -i ${filename}.t -o f${firmArgs##-}_${filename} ${firmArgs};
		fi
	fi
	rm -rf  ${filename}.t
}

k230_gzip()
{
	local filename="$1"
	local k230_gzip_tool="${K230_SDK_ROOT}/tools/k230_priv_gzip "
	${k230_gzip_tool} -n8  -f -k ${filename}  ||   ${k230_gzip_tool} -n9 -f -k ${filename} ||  \
	${k230_gzip_tool} -n7 -f -k ${filename}   ||   ${k230_gzip_tool} -n6 -f -k ${filename} || \
	${k230_gzip_tool} -n5 -f -k ${filename}   ||   ${k230_gzip_tool} -n4 -f -k ${filename}
	sed -i -e "1s/\x08/\x09/"  ${filename}.gz
}

# "-O linux -T firmware  -a ${add} -e ${add} -n ${name}"  
# "-n/-a/-s"
#file_gzip_ubootHead_firmHead ${quick_boot_cfg_data_file}  "-O linux -T firmware  -a ${add} -e ${add} -n ${name}"   "-n" 
#output fn_ug_xx fa_ug_xx fs_ug_xx;
bin_gzip_ubootHead_firmHead()
{
	local mkimage="${UBOOT_BUILD_DIR}/tools/mkimage"
	local file_full_path="$1"
	local filename=$(basename ${file_full_path})
	local mkimgArgs="$2"
	local firmArgs="$3"

	#[ -f ${file_full_path} ] || (echo ${filename} >${file_full_path} )
	# cd  "${BUILD_DIR}/images/";
	[ "$(dirname ${file_full_path})" == "$(pwd)" ] || cp ${file_full_path} .

	k230_gzip ${filename}

	#add uboot head 
	${mkimage} -A riscv -C gzip  ${mkimgArgs} -d ${filename}.gz  ug_${filename} # ${filename}.gzu; 

	add_firmHead ug_${filename}
	rm -rf ${filename}  ${filename}.gz ug_${filename}
} 

gen_uboot_bin()
{
	if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ] && [ "${CONFIG_SUPPORT_LINUX}" != "y" ]; then
        CONFIG_MEM_LINUX_SYS_BASE="${CONFIG_MEM_RTT_SYS_BASE}"
    fi;
	mkdir -p "${BUILD_DIR}/images/little-core/uboot"
	cd ${BUILD_DIR}/images/little-core/uboot;
	# "-O linux -T firmware  -a ${add} -e ${add} -n ${name}"  
	# "-n/-a/-s"  "-n/-a/-s"   
	#fn_ug_xxx  
	bin_gzip_ubootHead_firmHead  ${BUILD_DIR}/little/uboot/u-boot.bin   \
					"-O u-boot -T firmware  -a ${CONFIG_MEM_LINUX_SYS_BASE} -e ${CONFIG_MEM_LINUX_SYS_BASE} -n uboot" 

	
	cp ${BUILD_DIR}/little/uboot/spl/u-boot-spl.bin  . 
	add_firmHead  u-boot-spl.bin #
	${K230_SDK_ROOT}/src/little/uboot/tools/endian-swap.py   fn_u-boot-spl.bin  swap_fn_u-boot-spl.bin

	rm -rf u-boot-spl.bin
}


#生成可用uboot引导的linux版本文件
gen_linux_bin ()
{
	local mkimage="${UBOOT_BUILD_DIR}/tools/mkimage"
	local LINUX_SRC_PATH="src/little/linux"
	local LINUX_DTS_PATH="src/little/linux/arch/riscv/boot/dts/kendryte/${CONFIG_LINUX_DTB}.dts"
	
	cd  "${BUILD_DIR}/images/little-core/" ; 
	cpp -nostdinc -I ${K230_SDK_ROOT}/${LINUX_SRC_PATH}/include -I ${K230_SDK_ROOT}/${LINUX_SRC_PATH}/arch  -undef -x assembler-with-cpp ${K230_SDK_ROOT}/${LINUX_DTS_PATH}  hw/k230.dts.txt
	

	ROOTFS_BASE=`cat hw/k230.dts.txt | grep initrd-start | awk -F " " '{print $4}' | awk -F ">" '{print $1}'`
	ROOTFS_SIZE=`ls -lt rootfs-final.cpio.gz | awk '{print $5}'`
	((ROOTFS_END= $ROOTFS_BASE + $ROOTFS_SIZE))
	ROOTFS_END=`printf "0x%x" $ROOTFS_END`
	sed -i "s/linux,initrd-end = <0x0 .*/linux,initrd-end = <0x0 $ROOTFS_END>;/g" hw/k230.dts.txt

	${LINUX_BUILD_DIR}/scripts/dtc/dtc -I dts -O dtb hw/k230.dts.txt  >k230.dtb;		
	k230_gzip fw_payload.bin;
	echo a>rd;
	${mkimage} -A riscv -O linux -T multi -C gzip -a ${CONFIG_MEM_LINUX_SYS_BASE} -e ${CONFIG_MEM_LINUX_SYS_BASE} -n linux -d fw_payload.bin.gz:rd:k230.dtb  ulinux.bin;

	add_firmHead  ulinux.bin 
	mv fn_ulinux.bin  linux_system.bin
	[ -f fa_ulinux.bin ] && mv fa_ulinux.bin  linux_system_aes.bin
	[ -f fs_ulinux.bin ] && mv fs_ulinux.bin  linux_system_sm.bin
	rm -rf rd;
}

#生成ext2 格式镜像；
gen_final_ext2 ()
{
	local mkfs="${BUILDROOT_BUILD_DIR}/host/sbin/mkfs.ext4"
	cd  "${BUILD_DIR}/images/little-core/" ; 
	rm -rf rootfs.ext*
	rm -rf rootfs/dev/console
	rm -rf rootfs/dev/null

	fakeroot ${mkfs} -d rootfs  -r 1 -N 0 -m 1 -L "rootfs" -O ^64bit rootfs.ext4 80M
}


#生成可用uboot引导的rtt版本文件
gen_rtt_bin()
{
	local filename="fw_payload.bin"
	cd "${BUILD_DIR}/images/big-core/" ; 
	bin_gzip_ubootHead_firmHead "${BUILD_DIR}/common/big-opensbi/platform/kendryte/fpgac908/firmware/${filename}"  \
			 "-O opensbi -T multi  -a ${CONFIG_MEM_RTT_SYS_BASE} -e ${CONFIG_MEM_RTT_SYS_BASE} -n rtt"
	
	mv fn_ug_${filename}  rtt_system.bin
	[ -f fa_ug_${filename} ] && mv fa_ug_${filename}  rtt_system_aes.bin
	[ -f fs_ug_${filename} ] && mv fs_ug_${filename}  rtt_system_sm.bin
	chmod a+r rtt_system.bin;
}
#cfg_quick_boot,
#gen_part_bin quick_boot_cfg_data_file     name 0x80000 
gen_cfg_part_bin()
{
	local file_full_path="$1"
	local filename=$(basename ${file_full_path})
	local name="$2"
	local add="$3"
	mkdir -p  "${BUILD_DIR}/images/cfg_part";
	cd  "${BUILD_DIR}/images/cfg_part";
	mkdir -p ${GENIMAGE_CFG_DIR}/data
	[ -f ${file_full_path} ] || (echo ${filename} >${file_full_path} )

	bin_gzip_ubootHead_firmHead  ${file_full_path}   "-O linux -T firmware -a ${add} -e ${add} -n ${name}"
	
	rm -rf ${filename} 
}


#生成sd卡镜像文件
gen_image() 
{

	local genimage="${K230_SDK_ROOT}/tools/genimage "
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
	local all_kmode=""
	local size=""
	local start="0"

	cd ${BUILD_DIR}/images/big-core/; #cp  root/bin/test.kmodel   root/bin/test1.kmodel;
	rm -rf ai_mode ai_mode.bin; mkdir -p ai_mode; 
	if $(ls root/bin/*.kmodel >/dev/null 2>&1 )  ; then 
		cp  root/bin/*.kmodel ai_mode; 
		cd ai_mode;
		for file in $(ls *.kmodel) ; do 
			truncate -s %128 ${file};
			cat ${file} >> ../ai_mode.bin
			all_kmode="${all_kmode} ${file}";
			size=$(du -sb ${file} | cut -f1 )
			echo "${file%%\.*}_start=\"${start}\"" >>file_size.txt;
			echo "${file%%\.*}_size=\"${size}\"" >>file_size.txt;
			start=$((${start}+${size}))		
		done
	else 
		cd ai_mode;
	fi;
	
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
	cd ${BUILD_DIR}/images/big-core/;
	gen_image_spinor_proc_ai_mode;
	source ${BUILD_DIR}/images/big-core/ai_mode/file_size.txt
	

	#${K230_SDK_ROOT}/tools/genromfs -V ai -v -a 32 -f ai_mode.bin -d binbak/ 

	#rtapp and ai mode
	cd ${BUILD_DIR}/images/big-core/root/bin/;
	if [ -f  fastboot_app.elf ];then  fasb_app_size=$(du -sb fastboot_app.elf | cut -f1) ; cp  fastboot_app.elf ${BUILD_DIR}/images/big-core/;else fasb_app_size="0";fi
	# find . -type f  -not -name init.sh   | xargs rm -rf ; 
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


	cd ${K230_SDK_ROOT};make rtt_update_romfs;
	cd "${BUILD_DIR}/images/big-core/"
	cp ${BUILD_DIR}/big/rt-smart/rtthread.* .;
	
	bin_gzip_ubootHead_firmHead  ${BUILD_DIR}/common/big-opensbi/platform/kendryte/fpgac908/firmware/fw_payload.bin \
		   "-O opensbi -T multi -a ${CONFIG_MEM_RTT_SYS_BASE} -e ${CONFIG_MEM_RTT_SYS_BASE} -n rtt"
	mv  fn_ug_fw_payload.bin rtt_system.bin ;


	#gen_part_bin quick_boot_cfg_data_file     name 0x80000 
	gen_cfg_part_bin ${quick_boot_cfg_data_file}   quick_boot_cfg  ${CONFIG_MEM_QUICK_BOOT_CFG_BASE}
	gen_cfg_part_bin ${face_database_data_file}   face_db  ${CONFIG_MEM_FACE_DATA_BASE}
	gen_cfg_part_bin ${sensor_cfg_data_file}   sensor_cfg  ${CONFIG_MEM_SENSOR_CFG_BASE}
	gen_cfg_part_bin ${ai_mode_data_file}   ai_mode ${CONFIG_MEM_AI_MODEL_BASE}
	gen_cfg_part_bin ${speckle_data_file}  speckle ${CONFIG_MEM_SPECKLE_BASE}
	gen_cfg_part_bin ${rtapp_data_file}   rtapp  ${CONFIG_MEM_RTAPP_BASE}


	#生成spinor 镜像；
	gen_image ${GENIMAGE_CFG_SPI_NOR} sysimage-spinor32m.img
	return;
}
## 裁剪小核  ${BUILD_DIR}/images/little-core/rootfs/
## 裁剪大核  ${BUILD_DIR}/images/big-core/root/
shrink_rootfs_common()
{
	#裁剪小核rootfs
	cd ${BUILD_DIR}/images/little-core/rootfs/; 
	rm -rf lib/modules/;
	rm -rf lib/libstdc++*;
	rm -rf usr/bin/fio;
	rm -rf usr/bin/usb_test;
	rm -rf usr/bin/hid_gadget_test;
	rm -rf usr/bin/gadget*;
	rm -rf usr/bin/otp_test_demo;
	rm -rf usr/bin/iotwifi*;
	rm -rf usr/bin/i2c-tools.sh;
	rm -rf mnt/*;
	rm -rf app/;
	rm -rf lib/tuning-server;	
	rm -rf usr/bin/stress-ng  bin/bash usr/sbin/sshd usr/bin/trace-cmd usr/bin/lvgl_demo_widgets;
	rm -rf usr/lib/libcrypto.so.1.1 usr/bin/ssh  etc/ssh/moduli  usr/lib/libssl.so.1.1 usr/bin/ssh-keygen \
		usr/libexec/ssh-keysign  usr/bin/ssh-keyscan  usr/bin/ssh-add usr/bin/ssh-agent usr/libexec/ssh-pkcs11-helper\
		  usr/lib/libncurses.so.6.1 usr/lib/libvg_lite_util.so  usr/bin/par_ops usr/bin/sftp  usr/libexec/lzo/examples/lzotest;
	#find . -name *.ko | xargs rm -rf ;

	#裁剪大核rootfs;
	cd ${BUILD_DIR}/images/big-core/root/bin/;
	find . -type f  -not -name init.sh  -not -name  fastboot_app.elf    -not -name test.kmodel  | xargs rm -rf ; 

	if [ -f "${K230_SDK_ROOT}/src/big/mpp/userapps/src/vicap/src/isp/sdk/t_frameworks/t_database_c/calibration_data/sensor_cfg.bin" ]; then
        mkdir -p ${cfg_data_file_path};
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/src/vicap/src/isp/sdk/t_frameworks/t_database_c/calibration_data/sensor_cfg.bin ${cfg_data_file_path}/sensor_cfg.bin
	fi

}


gen_image_spinand()
{
	cd  ${BUILD_DIR}/;rm -rf images_bak;cp images images_bak -r;

	#裁剪小核rootfs
	cd ${BUILD_DIR}/images/little-core/rootfs/; 
	rm -rf lib/modules/;
	rm -rf lib/libstdc++*;
	rm -rf usr/bin/fio;
	rm -rf usr/bin/usb_test;
	rm -rf usr/bin/hid_gadget_test;
	rm -rf usr/bin/gadget*;
	rm -rf usr/bin/otp_test_demo;
	rm -rf usr/bin/iotwifi*;
	rm -rf usr/bin/i2c-tools.sh;
	rm -rf mnt/*;
	rm -rf app/;
	rm -rf lib/tuning-server;	
	rm -rf usr/bin/stress-ng  bin/bash usr/sbin/sshd usr/bin/trace-cmd;
	#find . -name *.ko | xargs rm -rf ;
	fakeroot -- ${K230_SDK_ROOT}/tools/mkcpio-rootfs.sh; 


	#裁剪大核romfs;
	#$(RT-SMART_SRC_PATH)/userapps/root
	#cd ${BUILD_DIR}/images/big-core/root/bin/; rm -rf sample_vicap_dump.elf sample_sys_init.elf  sample_venc.elf sample_dw200.elf sample_vdss.elf sample_vdd_r.elf sample_dpu* sample_dma* ./dpu;
	cd ${BUILD_DIR}/images/big-core/root/bin/;
	cd ${RTSMART_SRC_DIR}/userapps/; python3 ../tools/mkromfs.py ${BUILD_DIR}/images/big-core/root/  ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c;
	cd ${K230_SDK_ROOT}; make rtt_update_romfs;
	cd "${BUILD_DIR}/images/big-core/"
	cp ${BUILD_DIR}/big/rt-smart/rtthread.* .;
	
	bin_gzip_ubootHead_firmHead  ${BUILD_DIR}/common/big-opensbi/platform/kendryte/fpgac908/firmware/fw_payload.bin \
		   "-O opensbi -T multi -a ${CONFIG_MEM_RTT_SYS_BASE} -e ${CONFIG_MEM_RTT_SYS_BASE} -n rtt"
	mv  fn_ug_fw_payload.bin rtt_system.bin ;
	
	#生成spinor 镜像；
	gen_image ${GENIMAGE_CFG_SPI_NAND} sysimage-spinand32m.img

	#恢复删除前状态；
	cd  ${BUILD_DIR}/;rm -rf images_spinand;mv  images images_spinand; mv  images_bak images; 
	cp images_spinand/sysimage-spinand32m.img* images;
	return;
}

gen_env_bin()
{
	local mkenvimage="${UBOOT_BUILD_DIR}/tools/mkenvimage"
	cd  "${BUILD_DIR}/images/";
	local default_env_file=${env_dir}/default.env;
	local jffs2_env_file=${env_dir}/spinor.jffs2.env;
	local spinand_env_file=${env_dir}/spinand.env;

	sed -i -e "/^quick_boot/d"  ${jffs2_env_file}
	sed -i -e "/quick_boot/d"  ${spinand_env_file}
	sed -i -e "/^quick_boot/d"  ${default_env_file}
	sed -i -e "/restore_img/d"  ${default_env_file}

	if [ "${CONFIG_QUICK_BOOT}" != "y" ] || [ "${CONFIG_REMOTE_TEST_PLATFORM}" = "y" ] ; then 
		echo "quick_boot=false" >> ${jffs2_env_file}
		echo "quick_boot=false" >> ${spinand_env_file}
		echo "quick_boot=false" >> ${default_env_file}
	fi
	if [ "${CONFIG_REMOTE_TEST_PLATFORM}" = "y" ] ; then 
		echo "restore_img=mmc dev 0; mmc read 0x10000 0x200000 0x40000; gzwrite mmc 1 0x10000 0x8000000; reset" >> ${default_env_file}
	fi

	${mkenvimage} -s 0x10000 -o little-core/uboot/jffs2.env ${jffs2_env_file}
	${mkenvimage} -s 0x10000 -o little-core/uboot/spinand.env ${spinand_env_file}
	${mkenvimage} -s 0x10000 -o little-core/uboot/env.env  ${default_env_file}
}
copy_app()
{
	mkdir -p ${BUILD_DIR}/images/big-core/app
	if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ] ; then
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/*   ${BUILD_DIR}/images/big-core/app
		[ "${CONFIG_SUPPORT_LINUX}" = "y" ] && cp ${K230_SDK_ROOT}/src/common/cdk/user/out/big/*  ${BUILD_DIR}/images/big-core/app
	fi

	if [ "${CONFIG_REMOTE_TEST_PLATFORM}" = "y" ] ; then 
		local DIR="${K230_SDK_ROOT}/output/k230_evb_defconfig/images/k230_remote_test_platform"
		local SHAREFS="${DIR}/sharefs"
		rm ${DIR} -rf
		mkdir ${SHAREFS}/test_resource -p
		mkdir ${SHAREFS}/app -p

		cp /data1/share/test_resource_for_k230_cloud/* ${SHAREFS}/test_resource/ -rf

		cp ${K230_SDK_ROOT}/src/common/cdk/user/out/big/sample_receiver.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/common/cdk/user/out/big/sample_sys_init.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/common/cdk/user/out/big/sample_writer.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_venc.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_vdec.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_vo.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_mmz.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_vb.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_virtual_vio.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_dma.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_vicap.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_cipher.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_dpu.elf ${SHAREFS}/app/
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/elf/sample_audio.elf ${SHAREFS}/app/
	fi
}











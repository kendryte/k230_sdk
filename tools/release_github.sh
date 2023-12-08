#!/bin/bash
set -e

K230_SDK_ROOT="$(dirname $(dirname  $(readlink -f $0)))"
MPP_SRC_DIR="${K230_SDK_ROOT}/src/big/mpp"
CDK_SRC_DIR="${K230_SDK_ROOT}/src/common/cdk"
BUILDROOT_EXT_SRC_PATH="src/little/buildroot-ext"

RELEASE_K230_SDK_ROOT="$(dirname $(dirname  $(readlink -f $0)))/release_sdk"
RELEASE_MPP_SRC_DIR="${RELEASE_K230_SDK_ROOT}/src/big/mpp"
RELEASE_RTT_SRC_DIR="${RELEASE_K230_SDK_ROOT}/src/big/rt-smart"
RELEASE_CDK_SRC_DIR="${RELEASE_K230_SDK_ROOT}/src/common/cdk"
RELEASE_BUILDROOT_EXT_SRC_PATH="${RELEASE_K230_SDK_ROOT}/src/little/buildroot-ext"
RELEASE_RT_SMART_SRC_PATH="${RELEASE_K230_SDK_ROOT}/src/big/rt-smart"

open_mpp_kernel_name_list="!(\
include|\
sensor|\
connector|\
lib|\
fft|\
pm|\
mediafreq|\
Makefile|\
rt-smart.mk|\
mpp.mk\
)"

open_mpp_user_name_list="!(\
cipher|\
sensor|\
connector|\
fft|\
pm|\
Makefile|\
rt-smart.mk|\
mpp.mk\
)"

open_mpp_ioctl_name_list="!(\
k_ioctl.h|\
k_sensor_ioctl.h|\
k_connector_ioctl.h|\
k_fft_ioctl.h|\
k_pm_ioctl.h|\
)"

delete_cdk_user_name_list="\
user/component/ipcmsg/code \
user/component/ipcmsg/readme.txt \
user/component/ipcmsg/host/build \
user/component/ipcmsg/host/Makefile \
user/component/ipcmsg/slave/build \
user/component/ipcmsg/slave/Makefile \
user/component/datafifo/code \
user/component/datafifo/readme.txt \
user/component/datafifo/host/build \
user/component/datafifo/host/Makefile \
user/component/datafifo/slave/build \
user/component/datafifo/slave/Makefile \
user/mapi/mediaclient/lib/*.a \
user/mapi/mediaclient/lib/*.a \
user/out \
"

#open_cdk_kernel_name_list="!(\
#out\
#)"


open_tools_name_list="!(\
docker|\
doxygen|\
gen_image_cfg|
gen_image_script|\
kconfig|\
firmware_gen.py|\
mkcpio-rootfs.sh|\
k230_priv_gzip|\
post_copy_rootfs|\
ota|\
get_download_url.sh|\
menuconfig_to_code.sh|\
)"


#echo ${K230_SDK_ROOT};
#echo ${RELEASE_K230_SDK_ROOT};
#echo ${open_tools_name_list};

mkdir -p ${RELEASE_K230_SDK_ROOT};
rm ${RELEASE_K230_SDK_ROOT}/* -rf;
cd ${K230_SDK_ROOT};git checkout  -- configs/;
cp -rf `ls ${K230_SDK_ROOT}|grep -v release_sdk|grep -v output|grep -v toolchain|xargs`  ${RELEASE_K230_SDK_ROOT}/;
cp Kconfig.toolchain ${RELEASE_K230_SDK_ROOT}/;
cd ${RELEASE_K230_SDK_ROOT};
rm src/.src_fetched;

#mpp
(
	set -e;

	cd ${RELEASE_MPP_SRC_DIR}/kernel;
	mv MakefileOpen Makefile;
	shopt -s extglob
	rm -rf ${open_mpp_kernel_name_list};
	shopt -u extglob

	cd ${RELEASE_MPP_SRC_DIR}/userapps/src;
	mv MakefileOpen Makefile;
	shopt -s extglob
	rm -rf ${open_mpp_user_name_list};
	shopt -u extglob
	rm -rf sensor/build sensor/dewarp/*.bin
	rm -rf fft/build fft/dewarp/*.bin

	cd ${RELEASE_MPP_SRC_DIR}/include/ioctl;
	shopt -s extglob
	rm -rf ${open_mpp_ioctl_name_list};
	shopt -u extglob

	rm -rf ${RELEASE_MPP_SRC_DIR}/userapps/sample/elf;
	rm -rf ${RELEASE_MPP_SRC_DIR}/userapps/sample/fastboot_elf;

	rm -rf ${RELEASE_MPP_SRC_DIR}/userapps/sample/sample_jamlink
)
#rtsmart
(
	rm -rf  ${RELEASE_RTT_SRC_DIR}/kernel/bsp/maix3/board/interdrv/jamlink
)
#cdk
(
	set -e ;
    cd ${RELEASE_CDK_SRC_DIR};
	rm -rf ${delete_cdk_user_name_list};
	find  . -name *.o | xargs rm -rf;
	find . -type d -empty -not -name . | xargs rm -rf ;
    find . -type d -empty -not -name . | xargs rm -rf ;
	sed -i '22,23d' user/component/datafifo/Makefile
	sed -i '16,17d' user/component/datafifo/Makefile

	sed -i '21,22d' user/component/ipcmsg/Makefile
	sed -i '15,16d' user/component/ipcmsg/Makefile

)


#buildroot
rm  -rf ${RELEASE_BUILDROOT_EXT_SRC_PATH}/dl.tar.gz ${RELEASE_BUILDROOT_EXT_SRC_PATH}/buildroot-* ${RELEASE_BUILDROOT_EXT_SRC_PATH}/dl;
rm  -rf ${RELEASE_BUILDROOT_EXT_SRC_PATH}/package/tuning-server

cd ${RELEASE_K230_SDK_ROOT};
#rm .git
find .  -name .git -type d  | xargs rm -rf ;
find .  -name .vscode -type d  | xargs rm -rf ;
find .  -name .gitlab -type d  | xargs rm -rf ;
find .  -name .gitignore -o  -name .gitlab-ci.yml  | xargs rm -rf ;
find  . -name *.o | xargs rm -rf;
find  . -name *.dblite | xargs rm -rf;

#rm ai
rm src/big/nncase -rf;

#rm utils
rm src/big/utils -rf;

#rm kmodel
rm src/big/kmodel -rf;

#rm tools
cd ${RELEASE_K230_SDK_ROOT}/tools;
shopt -s extglob
rm -rf ${open_tools_name_list};
shopt -u extglob
cd ${RELEASE_K230_SDK_ROOT};

# github
cp -rf ${K230_SDK_ROOT}/.github  ${RELEASE_K230_SDK_ROOT}/;
# .gitignore
sed '/src\//d' ${K230_SDK_ROOT}/.gitignore > ${RELEASE_K230_SDK_ROOT}/.gitignore
# .gitlab-ci.yml
cp -rf ${K230_SDK_ROOT}/.gitlab-ci_github.yml  ${RELEASE_K230_SDK_ROOT}/.gitlab-ci.yml

rm -rf .config defconfig;
sed -i '/FETCH_CODE/d' Makefile;
make cdk-user-clean
make cdk-kernel-clean
make rt-smart-clean

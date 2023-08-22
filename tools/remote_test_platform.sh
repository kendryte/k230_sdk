#!/bin/bash
set -e

DIR="${K230_SDK_ROOT}/output/k230_evb_defconfig/images/k230_remote_test_platform"
SHAREFS="${DIR}/sharefs"
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
#!/bin/bash
cd ../
git checkout .
git pull
cd src/big/rt-smart
git checkout .
cd ../../big/mpp
git checkout .
cd ../../big/unittest
git checkout .
cd ../../common/cdk
git checkout .
cd ../../common/opensbi
git checkout .
cd ../../little/buildroot-ext
git checkout .
cd ../../little/linux
git checkout .
cd ../../little/uboot
git checkout .
cd ../../../
make prepare_sourcecode

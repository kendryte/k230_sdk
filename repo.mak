RT-SMART_VERSION        = "8c1a5e95ad947cdc729103ca8493e3baf6f39e01"
MPP_VERSION             = "88b478b0490fd3dc93e7f8617da6c37a765a61c9"
UNITTEST_VERSION        = "74b27c660ff3766df0256008101a20b7b800b7f8"
OPENSBI_VERSION         = "9aa2f6d34e685bbbb0afaab4e308b93a9cd06ec7"
BUILDROOT-EXT_VERSION   = "5deac453177420a38aa35c566a2c61ccc5bf8a82"
LINUX_VERSION           = "fcbe3a9531fcbb9e9201e3fe6edb1b6497529dee"
UBOOT_VERSION           = "37e542b5468b6b29f3293f111e154fcc74a16497"
CDK_VERSION             = "6d39496e72d92154b5c9aa4740e5729876af38bd"

RT-SMART_SRC_PATH        = src/big/rt-smart
MPP_SRC_PATH             = src/big/mpp
UNITTEST_SRC_PATH        = src/big/unittest
OPENSBI_SRC_PATH         = src/common/opensbi
BUILDROOT-EXT_SRC_PATH   = src/little/buildroot-ext
LINUX_SRC_PATH           = src/little/linux
UBOOT_SRC_PATH           = src/little/uboot
CDK_SRC_PATH             = src/common/cdk

RT-SMARTR_URL       = "git@g.a-bug.org:maix_sw/maix3_rt_smart.git"
MPP_URL             = "git@g.a-bug.org:maix_sw/mpp.git"
UNITTEST_URL        = "git@g.a-bug.org:maix_sw/maix3_unittest.git"
OPENSBI_URL         = "git@g.a-bug.org:maix_sw/maix3_opensbi.git"
BUILDROOT-EXT_URL   = "git@g.a-bug.org:maix_sw/buildroot-ext.git"
LINUX_URL           = "git@g.a-bug.org:maix_sw/maix3_linux_kernel.git"
UBOOT_URL           = "git@g.a-bug.org:maix_sw/maix3_u-boot.git"
CDK_URL             = "git@g.a-bug.org:maix_sw/cdk.git"

define FETCH_CODE
	set -e; \
	(test -d $(2) || ( \
		echo "clone $(1) into $(2), version $(3)";\
		git clone $(1) $(2) >/dev/null 2>&1;\
		cd $(2);\
		git checkout $(3) >/dev/null 2>&1;\
	)) && ( \
		echo "fetch $(1), version $(3)";\
		cd $(2);\
		git fetch;\
		git checkout $(3) >/dev/null 2>&1;\
	); \
	cd - >/dev/null 2>&1;\
	echo "success"
endef

RT-SMART_VERSION        = "e50e6e22d73cad35087f02bde02fd63e848e4ca4"
MPP_VERSION             = "186c38cf7fe38ccc3d580478629797941ed8f3b2"
UNITTEST_VERSION        = "74b27c660ff3766df0256008101a20b7b800b7f8"
OPENSBI_VERSION         = "2ef163a1773aaa6441fd0140d43b8f0e5f36165e"
BUILDROOT-EXT_VERSION   = "deea527b41f94a3c037cc10c4cd2289c2d01ec40"
LINUX_VERSION           = "f39b857553ceb3f7c0603b6b150494b984bdecaa"
UBOOT_VERSION           = "f953578ca5324bf8ec12529f99064822ab7d5822"
CDK_VERSION             = "a310b209c04dacb4bcccc931e981d5e5b05e2d9e"

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

RT-SMART_VERSION        = "1a25d7e49da28ef7f06334f3344c8587768ef1d4"
MPP_VERSION             = "7b75e867ab4c940d5ee553a67ee4a4c59c90701a"
UNITTEST_VERSION        = "a07caf5e19a6a215605271cfeedce18111790e9d"
OPENSBI_VERSION         = "b3a7a73c46a39e3e07209462b5c22d936b0fddca"
BUILDROOT-EXT_VERSION   = "a8dc2e9b4c47656db8c97307472c0763d598aba4"
LINUX_VERSION           = "fb13e7f92a34deb39b3578be341a5b11c5a4a2fb"
UBOOT_VERSION           = "faf1a8311cf4559fa8ffb166ba14099ee47bbae9"
CDK_VERSION             = "5ce32d38813e34c6cb97d0966d6999c210881464"

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

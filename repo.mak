RT-SMART_VERSION        = "fdeea349b50c985c0c4264a6d02fb7cec9ed5787"
MPP_VERSION             = "d7ec0c9eeae092906db1061f292502dae2e5d286"
UNITTEST_VERSION        = "a07caf5e19a6a215605271cfeedce18111790e9d"
OPENSBI_VERSION         = "fcbb4027b6e1f89f52d9079483e5199f30558f5b"
BUILDROOT-EXT_VERSION   = "adb839c7df7c9b9410159b4f9c57b299a2c38a2ev"
LINUX_VERSION           = "57d3d53e66d36923ba95b2fa7941ed37551b48e4"
UBOOT_VERSION           = "2580385862e86d308bad70d201772c50f750480e"
CDK_VERSION             = "40a7755ce12891fb5bd5379c09b73f8eb8fb3240"

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

RT-SMART_VERSION        = "0a2c55975d1181e66abf4ee10f8cf29f0966071d"
MPP_VERSION             = "0955ee6e157771ff8870bf49fd0c598126abe9f2"
UNITTEST_VERSION        = "a07caf5e19a6a215605271cfeedce18111790e9d"
OPENSBI_VERSION         = "9aa2f6d34e685bbbb0afaab4e308b93a9cd06ec7"
BUILDROOT-EXT_VERSION   = "8eafd41f1dde4ff48b914d5978cecf7af627fba6"
LINUX_VERSION           = "04b6371ff02d49cba18adc81dfcdbbec0cbfed56"
UBOOT_VERSION           = "317894980fd498b863820b1343096fe188faae53"
CDK_VERSION             = "fea98217f3a781508de19bb2ecfea390526414ba"

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

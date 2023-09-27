RT-SMART_VERSION        = "4d971dd2d3aee19908e1fd04053327ba627180a8"
MPP_VERSION             = "cb1f1c9607fdf7fc7d04d2dfa99bd73fbadec551"
UNITTEST_VERSION        = "74b27c660ff3766df0256008101a20b7b800b7f8"
OPENSBI_VERSION         = "9aa2f6d34e685bbbb0afaab4e308b93a9cd06ec7"
BUILDROOT-EXT_VERSION   = "1c4a01ef7f213a5b1e239185a63147f32a1745a2"
LINUX_VERSION           = "c5fd684f691c6822112e215bb4b7e7d3edfd20bc"
UBOOT_VERSION           = "b53d70c2137a34d55446cbbc506f3a3f23c3f06e"
CDK_VERSION             = "283f8759a8e156d957d7d8c93838bc09222e2460"
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

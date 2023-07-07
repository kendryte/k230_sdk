##############################CONFIGURATION##########################
CFG_3518EV300 = n
CFG_T31 = n
CFG_K510 ?= n
CFG_K230 ?= y

SOCCHANNEL_DEBUG = y

###############################PLATFORM##############################
WLAN_CFLAGS +=-D_PRE_PLATFORM_JZ=1
WLAN_CFLAGS +=-D_PRE_PLATFORM_SOC=2
WLAN_CFLAGS +=-D_PRE_PLATFORM_CANAAN=3
WLAN_CFLAGS +=-D_PRE_OS_PLATFORM=_PRE_PLATFORM_CANAAN

ifeq ($(SOCCHANNEL_DEBUG), y)
WLAN_CFLAGS += -D_PRE_SOCCHANNEL_DEBUG
endif

################################INCLUDE##############################
WLAN_CFLAGS += -I$(WLAN_DIR)/oal
WLAN_CFLAGS += -I$(WLAN_DIR)/wal
WLAN_CFLAGS += -I$(WLAN_DIR)/hcc
WLAN_CFLAGS += -I$(WLAN_DIR)/inc
WLAN_CFLAGS += -I$(WLAN_DIR)/oam
WLAN_CFLAGS += -I$(WLAN_DIR)/libsec

################################KERNEL_VERSION########################
WLAN_CFLAGS +=-D_PRE_KERVER_4D9=1
WLAN_CFLAGS +=-D_PRE_KERVER_4D17=2
WLAN_CFLAGS +=-D_PRE_KERVER_5D10=3

ifeq ($(CFG_K230), y)
WLAN_CFLAGS +=-D_PRE_KERVER=_PRE_KERVER_5D10
else ifeq ($(CFG_K510), y)
WLAN_CFLAGS +=-D_PRE_KERVER=_PRE_KERVER_4D17
else
WLAN_CFLAGS +=-D_PRE_KERVER=_PRE_KERVER_4D9
endif
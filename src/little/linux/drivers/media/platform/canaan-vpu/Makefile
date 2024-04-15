#############################################if/##############
# Set the include-path according to the defined interface.
###########################################################

ccflags-y += -I$(src) -I$(src)/if -I$(src)/dev -I$(src)/if/v4l2 -I$(src)/external
ccflags-$(CONFIG_VPU_CANAAN_FTRACE) += -DMVX_LOG_FTRACE_ENABLE
ccflags-$(CONFIG_VPU_CANAAN_PRINT_FILE) += -DMVX_LOG_PRINT_FILE_ENABLE
ccflags-y += $(EXTRA_CCFLAGS)

###########################################################
# Define build targets and what files to include.
###########################################################

# Amvx module
obj-$(CONFIG_VPU_CANAAN) := amvx.o

# Add objects for if module.
if-y := mvx_if.o \
	mvx_buffer.o \
	mvx_firmware_cache.o \
	mvx_firmware.o \
	mvx_firmware_v2.o \
	mvx_firmware_v3.o \
	mvx_mmu.o \
	mvx_secure.o \
	mvx_session.o \
	fw_jpegenc.o \
	fw_jpegdec.o \
	fw_h264enc.o \
	fw_h264dec.o \
	fw_hevcenc.o \
	fw_hevcdec.o

# Add external interface.
if-y += mvx_ext_v4l2.o \
	mvx_v4l2_buffer.o \
	mvx_v4l2_session.o \
	mvx_v4l2_vidioc.o \
	mvx_v4l2_fops.o \
	mvx_v4l2_ctrls.o

# Add objects for dev module.
dev-y := mvx_dev.o \
	 mvx_hwreg.o \
	 mvx_hwreg_v500.o \
	 mvx_hwreg_v550.o \
	 mvx_hwreg_v61.o \
	 mvx_hwreg_v52_v76.o \
	 mvx_lsid.o \
	 mvx_scheduler.o \
	 mvx_pm_runtime.o

# Add driver objects.
amvx-y := mvx_driver.o \
	  mvx_seq.o \
	  mvx_log.o \
	  mvx_log_group.o \
	  $(if-y) $(dev-y)

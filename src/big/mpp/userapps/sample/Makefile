MPP_BUILD_DIR ?= $(shell pwd)
FASTBOOT_BUILD_DIR=$(MPP_BUILD_DIR)/userapps/sample/fastboot_app
TRIPLE_CAMERA_BUILD_DIR=$(MPP_BUILD_DIR)/userapps/sample/sample_triple_camera_facedetect
all:
	@rm -f elf/*
	@mkdir -p $(FASTBOOT_BUILD_DIR)
	@mkdir -p $(TRIPLE_CAMERA_BUILD_DIR)
	@cd sample_audio; make || exit 1
	@cd sample_dma; make || exit 1
	@cd sample_mmz; make || exit 1
	@cd sample_nonai_2d; make || exit 1
	@cd sample_csc; make || exit 1
	@cd sample_vb; make || exit 1
	@cd sample_vdec; make || exit 1
	@cd sample_venc; make || exit 1
	@cd sample_virtual_vio; make || exit 1
	@cd sample_vo; make || exit 1
	@cd sample_vdss; make || exit 1
	@cd sample_dpu; make || exit 1
	@cd sample_dma_bind; make || exit 1
	@cd sample_vicap; make || exit 1
	@cd sample_gpio; make || exit 1
	@cd sample_mcm; make || exit 1
	@cd sample_adc; make || exit 1
	@cd sample_dw200; make || exit 1
	@cd sample_log; make || exit 1
	@cd sample_cipher/sample_hash; make || exit 1
	@cd sample_cipher/sample_hwhash; make || exit 1
	@cd sample_cipher/sample_aes; make || exit 1
	@cd sample_cipher/sample_sm4; make || exit 1
	@cd sample_vdd_r; make || exit 1
	@cd sample_wdt; make || exit 1
	@cd sample_av; make || exit 1
	@cd sample_otp; make || exit 1
	@cd sample_ts; make || exit 1
	@cd sample_vdv; make || exit 1
	@cd sample_face_ae; make || exit 1
	@cd sample_face_detect; make || exit 1
	@cd fastboot_app; mkdir -p $(FASTBOOT_BUILD_DIR); cd $(FASTBOOT_BUILD_DIR); cmake $(MPP_SRC_DIR)/userapps/sample/fastboot_app;  make && make install;
	@cd sample_dpu_vicap; make || exit 1
	@cd sample_triple_camera_facedetect; mkdir -p $(TRIPLE_CAMERA_BUILD_DIR); cd $(TRIPLE_CAMERA_BUILD_DIR); cmake $(MPP_SRC_DIR)/userapps/sample/sample_triple_camera_facedetect;  make && make install;
	@cd sample_dpu_vo; make || exit 1
	@cd sample_pwm; make || exit 1
	@cd sample_pm; make || exit 1
	@cd sample_fft; make || exit 1
	@cd sample_canaan_uart_rx; make || exit 1
	@cd sample_canaan_uart_tx; make || exit 1
	#@cd opencv_camera_test; make || exit 1
clean:
	@cd sample_audio; make clean
	@cd sample_dma; make clean
	@cd sample_mmz; make clean
	@cd sample_nonai_2d; make clean
	@cd sample_csc; make clean
	@cd sample_vb; make clean
	@cd sample_vdec; make clean
	@cd sample_venc; make clean
	@cd sample_virtual_vio; make clean
	@cd sample_vo; make clean
	@cd sample_vdss; make clean
	@cd sample_dpu; make clean
	@cd sample_dma_bind; make clean
	@cd sample_vicap; make clean
	@cd sample_mcm; make clean
	@cd sample_gpio; make clean
	@cd sample_adc; make clean
	@cd sample_dw200; make clean
	@cd sample_log; make clean
	@cd sample_cipher/sample_hash; make clean
	@cd sample_cipher/sample_hwhash; make clean
	@cd sample_cipher/sample_aes; make clean
	@cd sample_cipher/sample_sm4; make clean
	@cd sample_vdd_r; make clean
	@cd sample_wdt; make clean
	@cd sample_av; make clean
	@cd sample_otp; make clean
	@cd sample_ts; make clean
	@cd sample_vdv; make clean
	@cd sample_face_ae; make clean
	@cd sample_face_detect; make clean
#	@cd fastboot_app; make clean
	@cd sample_dpu_vicap; make clean
	@cd sample_dpu_vo; make clean
	@cd sample_pwm; make clean
	@cd sample_pm; make clean
	@cd sample_fft; make clean
	@cd sample_canaan_uart_rx; make clean
	@cd sample_canaan_uart_tx; make clean




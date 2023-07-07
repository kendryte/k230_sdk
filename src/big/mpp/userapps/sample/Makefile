all:
	@rm -f elf/*
	@cd sample_audio; make || exit 1
	@cd sample_dma; make || exit 1
	@cd sample_mmz; make || exit 1
	@cd sample_nonai_2d; make || exit 1
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
	@cd sample_adc; make || exit 1
	@cd sample_dw200; make || exit 1
	@cd sample_log; make || exit 1
	@cd sample_cipher; make || exit 1
	@cd sample_vdd_r; make || exit 1
	@cd sample_wdt; make || exit 1
	@cd sample_av; make || exit 1
	@cd sample_otp; make || exit 1
	@cd sample_ts; make || exit 1
	@cd sample_vdv; make || exit 1
	@cd sample_face_ae; make || exit 1
	@cd fastboot_app; cmake . && make;make install || exit 1
	@cd sample_dpu_vicap; make || exit 1
	@cd sample_dpu_vo; make || exit 1
	@cd sample_pwm; make || exit 1
clean:
	@cd sample_audio; make clean
	@cd sample_dma; make clean
	@cd sample_mmz; make clean
	@cd sample_nonai_2d; make clean
	@cd sample_vb; make clean
	@cd sample_vdec; make clean
	@cd sample_venc; make clean
	@cd sample_virtual_vio; make clean
	@cd sample_vo; make clean
	@cd sample_vdss; make clean
	@cd sample_dpu; make clean
	@cd sample_dma_bind; make clean
	@cd sample_vicap; make clean
	@cd sample_gpio; make clean
	@cd sample_adc; make clean
	@cd sample_dw200; make clean
	@cd sample_log; make clean
	@cd sample_cipher; make clean
	@cd sample_vdd_r; make clean
	@cd sample_wdt; make clean
	@cd sample_av; make clean
	@cd sample_otp; make clean
	@cd sample_ts; make clean
	@cd sample_vdv; make clean
	@cd sample_face_ae; make clean
	@cd fastboot_app; make clean
	@cd sample_dpu_vicap; make clean
	@cd sample_dpu_vo; make clean
	@cd sample_pwm; make clean


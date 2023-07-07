ifndef CONFIG_SPL_BUILD
	ifndef  CONFIG_TARGET_K230_FPGA
	INPUTS-y += add_firmware_head
	endif  
endif 
add_firmware_head: u-boot.bin spl/u-boot-spl.bin u-boot.img 
	
	#增加bootrom头
	cp spl/u-boot-spl.bin  t.bin ; python3 $(srctree)/tools/firmware_gen.py  -i t.bin -o u-boot-spl-k230.bin -n
	$(srctree)/tools/endian-swap.py   u-boot-spl-k230.bin  u-boot-spl-k230-swap.bin
	cp spl/u-boot-spl.bin  t.bin ; python3 $(srctree)/tools/firmware_gen.py  -i t.bin -o u-boot-spl-k230_aes.bin -a
	$(srctree)/tools/endian-swap.py   u-boot-spl-k230.bin  u-boot-spl-k230_aes-swap.bin
	cp spl/u-boot-spl.bin  t.bin ; python3 $(srctree)/tools/firmware_gen.py  -i t.bin -o u-boot-spl-k230_sm.bin -s
	$(srctree)/tools/endian-swap.py   u-boot-spl-k230.bin  u-boot-spl-k230_sm-swap.bin
	
	#生成sd卡非安全镜像
	dd if=u-boot-spl-k230.bin of=sd.iso bs=512 seek=$$((0x100000/512))
	dd if=u-boot.img of=sd.iso bs=512 seek=$$((0x200000/512))

	# #生成sd卡aes镜像
	dd if=u-boot-spl-k230_aes.bin of=sd_aes.iso bs=512 seek=$$((0x100000/512))
	dd if=u-boot.img of=sd_aes.iso bs=512 seek=$$((0x200000/512))

	# #生成sd卡sm镜像
	dd if=u-boot-spl-k230_sm.bin of=sd_sm.iso bs=512 seek=$$((0x100000/512))
	dd if=u-boot.img of=sd_sm.iso bs=512 seek=$$((0x200000/512))


	rm -rf t.bin;
	ls -lht   spl/u-boot-spl  u-boot  sd.iso  sd_aes.iso  sd_sm.iso u-boot-spl-k230.bin    u-boot.img
	echo "sd image file is sd.iso"
	

#spl_k230.bin:
#k230_uboot.iso:

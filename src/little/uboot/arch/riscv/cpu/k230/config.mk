ifndef CONFIG_SPL_BUILD
	ifndef  CONFIG_TARGET_K230_FPGA
	INPUTS-y += add_firmware_head
	endif  
endif 
add_firmware_head: u-boot.bin spl/u-boot-spl.bin u-boot.img 
	#给一级boot增加bootrom头
	cp spl/u-boot-spl.bin  t.bin ; python3 $(srctree)/tools/firmware_gen.py  -i t.bin -o u-boot-spl-k230.bin -n
	$(srctree)/tools/endian-swap.py   u-boot-spl-k230.bin  u-boot-spl-k230-swap.bin

	#二级uboot 压缩、uboot头、firmware头
	gzip -k -f u-boot.bin ;
	./tools/mkimage -A riscv -O u-boot -C gzip -T firmware -a ${CONFIG_SYS_TEXT_BASE} -e ${CONFIG_SYS_TEXT_BASE} -n uboot -d  u-boot.bin.gz   u-boot.img
	cp u-boot.img t.bin;python3 $(srctree)/tools/firmware_gen.py  -i  t.bin -o fn_u-boot.img -n


	# cp spl/u-boot-spl.bin  t.bin ; python3 $(srctree)/tools/firmware_gen.py  -i t.bin -o u-boot-spl-k230_aes.bin -a
	# $(srctree)/tools/endian-swap.py   u-boot-spl-k230.bin  u-boot-spl-k230_aes-swap.bin
	# cp spl/u-boot-spl.bin  t.bin ; python3 $(srctree)/tools/firmware_gen.py  -i t.bin -o u-boot-spl-k230_sm.bin -s
	# $(srctree)/tools/endian-swap.py   u-boot-spl-k230.bin  u-boot-spl-k230_sm-swap.bin
	
	#生成sd卡非安全镜像
	dd if=u-boot-spl-k230.bin of=sd.iso bs=512 seek=$$((0x100000/512))
	dd if=fn_u-boot.img of=sd.iso bs=512 seek=$$((0x200000/512))


	

#spl_k230.bin:
#k230_uboot.iso:

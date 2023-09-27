# /* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
#  *
#  * Redistribution and use in source and binary forms, with or without
#  * modification, are permitted provided that the following conditions are met:
#  * 1. Redistributions of source code must retain the above copyright
#  * notice, this list of conditions and the following disclaimer.
#  * 2. Redistributions in binary form must reproduce the above copyright
#  * notice, this list of conditions and the following disclaimer in the
#  * documentation and/or other materials provided with the distribution.
#  *
#  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
#  * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#  * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#  * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
#  * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  */

obj-$(CONFIG_LPDDR4_1066) += lpddr4_init_32_swap_1066.o
obj-$(CONFIG_LPDDR4_3200) += lpddr4_init_32_swap_3200.o
obj-$(CONFIG_LPDDR4_2667) += gen_lpddr4_2667.o



##ddr size
$(obj)/gen_lpddr4_2667.c: $(src)/lpddr4_init_32_swap_2667.c    $(srctree)/arch/riscv/cpu/k230/ddr.sh
	($(srctree)/arch/riscv/cpu/k230/ddr.sh $< $@ 0x50000 0x53fff 0x54000 0x5433d  || exit 1)
ifeq ($(CONFIG_DDR_SIZE),0x80000000) 
	sed -i  -E -e 's/(^[^/]*0x00000218[ ]*,)([^)]*)(.*)/\1 0x07070707 \3 \/\/have modify /'  $@ ; 
else ifeq ($(CONFIG_DDR_SIZE),0x20000000)  
	sed -i  -E -e 's/(^[^/]*0x00000218[ ]*,)([^)]*)(.*)/\1 0x0f0f0707 \3/ \/\/have modify'  $@ ; 
endif 
#sed -n -E -e 's/(^[^/]*0x00000218[ ]*,)([^)]*)(.*)/\1 0x07070707 \3/p'  $@ 
#sed -n -E -e 's/(^[^/]*0x00000218[ ]*,)([^)]*)(.*)/\1 0x0f0f0707 \3/p'  $@ 
#0x0f0f0f07  256MB;
#0x0f0f0707 512MB
#0x07070707 2GB



# obj-$(CONFIG_SIPLP4_3200_WODT) += siplp4_3200_hwodt.o
# obj-$(CONFIG_SIPLP4_3200_WALLODT) += siplp4_3200_allodt.o

# #arch/riscv/cpu/k230/ddr4/sip_lpddr4_init_32_swap_3200_with_wodt.c
# $(obj)/siplp4_3200_hwodt.c: $(src)/sip_lpddr4_init_32_swap_3200_with_wodt.c   
# 	($(srctree)/arch/riscv/cpu/k230/ddr_2d.sh $< $@ 0x50000 0x53fff 0x54000 0x5433d  0x542bd || exit 1)

# $(obj)/siplp4_3200_allodt.c: $(src)/sip_lpddr4_init_32_swap_3200_with_allodt.c
# 	($(srctree)/arch/riscv/cpu/k230/ddr_2d.sh $< $@ 0x50000 0x53fff 0x54000 0x5433d  0x542bd || exit 1)




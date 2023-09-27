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

obj-y += board.o


obj-$(CONFIG_LPDDR3_800) +=  gen_lpddr3_800.o    #800
obj-$(CONFIG_LPDDR3_1600) += gen_lpddr3_1600.o   #1600
obj-$(CONFIG_LPDDR3_2133) += gen_lpddr3_2133.o   #ddr_init_2133.c
# obj-$(CONFIG_CANMV_LPDDR3_2133) += gen_pilpddr3_2133.o   #ddr_init_2133.c



$(obj)/gen_lpddr3_2133.c: $(src)/ddr_init_2133.c   $(srctree)/arch/riscv/cpu/k230/ddr.sh
	($(srctree)/arch/riscv/cpu/k230/ddr.sh $< $@ 0x50000 0x53fff 0x54000 0x54359  || exit 1)

$(obj)/gen_lpddr3_1600.c: $(src)/lpddr3_swap_1600.c   $(srctree)/arch/riscv/cpu/k230/ddr.sh
	($(srctree)/arch/riscv/cpu/k230/ddr.sh $< $@ 0x50000 0x53fff 0x54000 0x54359  || exit 1)

$(obj)/gen_lpddr3_800.c: $(src)/lpddr3_swap_800.c   $(srctree)/arch/riscv/cpu/k230/ddr.sh
	($(srctree)/arch/riscv/cpu/k230/ddr.sh $< $@ 0x50000 0x53fff 0x54000 0x54359  || exit 1)

	

EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_common  
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_crypto
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_dma
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ka
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_rt
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ecc
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_hmac
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ecp
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_sm2
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_sp38a
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_sp38d


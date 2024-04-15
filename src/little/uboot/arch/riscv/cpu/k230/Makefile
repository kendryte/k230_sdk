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

obj-y += dram.o
obj-y += cpu.o
obj-y += cache.o
obj-y += puf_fw/pufs_common/pufs_internal.o 
obj-y += puf_fw/pufs_common/pufs_reg_ctrl.o 
obj-y += puf_fw/pufs_crypto/pufs_crypto.o 
obj-y += puf_fw/pufs_dma/pufs_dma.o 
obj-y += puf_fw/pufs_ecc/pufs_ecc.o 
obj-y += puf_fw/pufs_ecp/pufs_ecp_mprog.o 
obj-y += puf_fw/pufs_ecp/pufs_ecp.o 
obj-y += puf_fw/pufs_hmac/pufs_hmac.o 
obj-y += puf_fw/pufs_ka/pufs_ka.o 
obj-y += puf_fw/pufs_rt/pufs_rt.o 
obj-y += puf_fw/pufs_rt/pufs_rt_cde.o 
obj-y += puf_fw/pufs_sm2/pufs_sm2.o 
obj-y += puf_fw/pufs_sp38a/pufs_sp38a.o 
obj-y += puf_fw/pufs_sp38d/pufs_sp38d.o 







obj-y += sysctl.o
obj-y += unzip.o
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_common  
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_crypto
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_dma
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ecc
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ecp
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_hmac
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ka
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_rt
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_sm2

obj-$(CONFIG_$(SPL_)DFU_VIRT) += dfu_otp.o

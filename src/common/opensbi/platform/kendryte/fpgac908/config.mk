#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Damien Le Moal <damien.lemoal@wdc.com>
#

# Compiler flags
platform-cppflags-y =
platform-cflags-y = -g
platform-asflags-y = -g
platform-ldflags-y = -g

# Blobs to build
PLATFORM_RISCV_ISA=rv64gcxthead
FW_TEXT_START=0x200000
FW_PAYLOAD=y
FW_PAYLOAD_OFFSET=0x20000
FW_JUMP_ADDR=0x220000

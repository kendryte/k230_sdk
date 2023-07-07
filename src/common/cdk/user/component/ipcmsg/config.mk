#
# OS configuration support linux or rt-smart
#
SERVER_OS_TYPE=rt-smart
CLIENT_OS_TYPE=linux

#
# Transport confgiuration support socket or ipcm
# Server and Client should use same transport
#
TRANSPORT=ipcm

#
# Platform configuration support x86 or riscv
# Please note that the x86 platform does not support rt-smart OS
#
SERVER_PLATFORM=riscv
CLIENT_PLATFORM=riscv

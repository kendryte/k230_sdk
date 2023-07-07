#
# Copyright (c) 2006-2020, RT-Thread Development Team
#
# SPDX-License-Identifier: GPL-2.0
#
# Change Logs:
# Date           Author       Notes
# 2020-10-20     Bernard      The first version
#

import os

class ARCHRISCV64():
    def __init__(self, **kwargs):
        configuration = kwargs.get('configuration', {})

        Toolchain = configuration.get('TOOLCHAIN', kwargs.get('Toolchain', 'gcc'))
        BUILD = configuration.get('BUILD', kwargs.get('BUILD', 'debug'))
        USR_ROOT = configuration.get('USR_ROOT', kwargs.get('USR_ROOT', '/'))

        LIBC_MODE = 'release' # 'debug' or 'release', if debug, use libc in components, or not use libc with toolchain

        if Toolchain.find('gcc') != -1:
            self.ARCH = 'risc-v'
            self.ARCH_CPU = 'rv64'
            self.PREFIX = configuration.get('PREFIX', kwargs.get('PREFIX', 'riscv64-unknown-linux-musl-'))
            self.CC = configuration.get('CC', self.PREFIX + 'gcc')
            self.CXX = configuration.get('CXX', self.PREFIX + 'g++')
            self.AS = self.PREFIX + 'gcc'
            self.AR = self.PREFIX + 'ar'
            self.LINK = self.PREFIX + 'gcc'
            self.TARGET_EXT = configuration.get('TARGET_EXT', 'elf')
            self.SIZE = self.PREFIX + 'size'
            self.OBJDUMP = self.PREFIX + 'objdump'
            self.OBJCPY = self.PREFIX + 'objcopy'
            self.STRIP = self.PREFIX + 'strip'
            self.EXEC_PATH = configuration.get('EXEC_PATH', kwargs.get('EXEC_PATH', 'NULL'))
            self.RANLIB = self.PREFIX + 'ranlib'

            if LIBC_MODE == 'debug':
                EXT_CFLAGS = ' -nostdinc -nostdlib -Isdk/libc/include'
                EXT_LFLAGS = ' -nostdlib -Lsdk/libc/lib -lcrt -lc -Wl,--no-whole-archive'
            else:
                EXT_CFLAGS = ''
                EXT_LFLAGS = ''

            DEVICE = ' -mcmodel=medany -march=rv64imafdcv -mabi=lp64d'
            self.CFLAGS    = configuration.get('CFLAGS', DEVICE + ' -Werror -Wall' + EXT_CFLAGS)
            self.AFLAGS    = configuration.get('AFLAGS', ' -c' + DEVICE + ' -x assembler-with-cpp -D__ASSEMBLY__ -I.' + EXT_CFLAGS)
            LINK_SCRIPT    = configuration.get('LINK_SCRIPT', os.path.join(USR_ROOT, 'linker_scripts', 'riscv64', 'link.lds'))
            # add ' -Lsdk/rt-thread/lib -Wl,--whole-archive -lrtthread -Wl,--no-whole-archive' to add strong symbol
            self.LFLAGS    = configuration.get('LFLAGS', DEVICE + ' -T %s' % LINK_SCRIPT  + EXT_LFLAGS + ' -Lsdk/rt-thread/lib -Wl,--whole-archive -lrtthread -Wl,--no-whole-archive')

            self.PIC_FLAGS     = configuration.get('PIC_FLAGS', ' -fPIC')
            self.STATIC_FLAGS  = configuration.get('STATIC_FLAGS', ' -n --static')
            self.SHARED_FLAGS  = configuration.get('SHARED_FLAGS', ' -shared')

            self.BUILD_DEBUG   = ' -O0 -g -gdwarf-2'
            self.BUILD_RELEASE = ' -O2 -Os'

            self.CXXFLAGS = configuration.get('CXXFLAGS', self.CFLAGS + ' -Wno-delete-non-virtual-dtor -fexceptions -frtti')
        else:
            print('unsupport toolchain')
            exit(-1)

if __name__ == '__main__':
    arch = ARCHRISCV64(Toochain = 'gcc')
    print(arch.CC)

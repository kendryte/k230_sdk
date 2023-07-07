#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# File      : building.py
# This file is part of RT-Thread RTOS
# COPYRIGHT (C) 2006 - 2019, RT-Thread Development Team
#
# Change Logs:
# Date           Author       Notes
# 2019-05-26     Bernard      The first version
#

import os
import sys
import string
import pdb

from SCons.Script import *
from building import *

def BuildHostApplication(TARGET, SConscriptFile):
    import platform
    global Env

    platform_type = platform.system()
    if platform_type == 'Windows' or platform_type.find('MINGW') != -1:
        TARGET = TARGET.replace('.mo', '.exe')

    HostRtt = os.path.join(os.path.dirname(__file__), 'host', 'rtthread')
    Env = Environment()

    if not GetOption('verbose'):
        # override the default verbose command string
        Env.Replace(
            ARCOMSTR = 'AR $TARGET',
            ASCOMSTR = 'AS $TARGET',
            ASPPCOMSTR = 'AS $TARGET',
            CCCOMSTR = 'CC $TARGET',
            CXXCOMSTR = 'CXX $TARGET',
            LINKCOMSTR = 'LINK $TARGET'
        )

    objs = SConscript(SConscriptFile)
    objs += SConscript(HostRtt + '/SConscript')

    target = Env.Program(TARGET, objs)
    return target

def BuildHostLibrary(TARGET, SConscriptFile):
    import platform
    global Env

    platform_type = platform.system()
    if platform_type == 'Windows' or platform_type.find('MINGW') != -1:
        TARGET = TARGET.replace('.mo', '.exe')

    HostRtt = os.path.join(os.getcwd(), 'tools', 'host', 'rtthread')
    Env = Environment()

    if not GetOption('verbose'):
        # override the default verbose command string
        Env.Replace(
            ARCOMSTR = 'AR $TARGET',
            ASCOMSTR = 'AS $TARGET',
            ASPPCOMSTR = 'AS $TARGET',
            CCCOMSTR = 'CC $TARGET',
            CXXCOMSTR = 'CXX $TARGET',
            LINKCOMSTR = 'LINK $TARGET'
        )

    objs = SConscript(SConscriptFile)
    objs += SConscript(HostRtt + '/SConscript')

    target = Env.Program(TARGET, objs)
    return target

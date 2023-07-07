#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
import sys
import string
import pdb

from SCons.Script import *

BuildOptions = {}
DependBuildOptions = {}
Projects = []
Env = None
ArchConfig = None

RTT_ROOT = ''
BSP_ROOT = ''
USR_ROOT = ''
GCC_MUSL = False

prefix = 'root/bin'

class Win32Spawn:
    def spawn(self, sh, escape, cmd, args, env):
        # deal with the cmd build-in commands which cannot be used in
        # subprocess.Popen
        if cmd == 'del':
            for f in args[1:]:
                try:
                    os.remove(f)
                except Exception as e:
                    print ('Error removing file: ' + e)
                    return -1
            return 0

        import subprocess

        newargs = ' '.join(args[1:])
        cmdline = cmd + " " + newargs

        # Make sure the env is constructed by strings
        _e = dict([(k, str(v)) for k, v in env.items()])

        # Windows(tm) CreateProcess does not use the env passed to it to find
        # the executables. So we have to modify our own PATH to make Popen
        # work.
        old_path = os.environ['PATH']
        os.environ['PATH'] = _e['PATH']

        try:
            proc = subprocess.Popen(cmdline, env=_e, shell=False)
        except Exception as e:
            print ('Error in calling command:' + cmdline.split(' ')[0])
            print ('Exception: ' + os.strerror(e.errno))
            if (os.strerror(e.errno) == "No such file or directory"):
                print ("\nPlease check Toolchains PATH setting.\n")

            return e.errno
        finally:
            os.environ['PATH'] = old_path

        return proc.wait()

def GetEnv():
    global Env

    return Env

def SetEnv(env):
    global Env
    Env = env

    Export('Env')

    return

def PrepareEnv(usr_root):
    global BuildOptions
    global Projects
    global ArchConfig
    global USR_ROOT
    global Env

    # reset variable
    BuildOptions = {}
    Projects = []
    ArchConfig = None

    USR_ROOT = LocateConfiguration(usr_root)

    if not Env:
        DefaultEnvironment(tools=[])

        # add comstr option
        AddOption('--verbose',
                    dest='verbose',
                    action='store_true',
                    default=False,
                    help='print verbose information during build')
        AddOption('--release',
                    dest='release',
                    action='store_true',
                    default=False,
                    help='release building')
        AddOption('--target',
                        dest = 'target',
                        type = 'string',
                        help = 'set target project: vsc')

    Env = None

# create the environment
def BuildEnv():
    from install import TOOL_INSTALL
    global USR_ROOT
    global ArchConfig

    # get toolchain configuration
    BuildConfiguration()

    DefaultEnvironment(tools=[])
    env = Environment(tools = ['gcc', 'g++', 'gnulink', 'ar'],
        AS   = ArchConfig.AS, ASFLAGS = ArchConfig.AFLAGS,
        CC   = ArchConfig.CC, CCFLAGS = ArchConfig.CFLAGS,
        CXX  = ArchConfig.CXX, CXXFLAGS = ArchConfig.CXXFLAGS,
        AR   = ArchConfig.AR, ARFLAGS = '-rc',
        RANLIB = ArchConfig.RANLIB,
        LINK = ArchConfig.LINK, LINKFLAGS = ArchConfig.LFLAGS)
    env.PrependENVPath('PATH', ArchConfig.EXEC_PATH)
    env['SHOBJSUFFIX'] = '.o'
    env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
    env['SHLIBPREFIX'] = 'lib'
    env['SHLIBSUFFIX'] = '.so'
    if 'ASPPCOM' in env:
        env['ASCOM'] = env['ASPPCOM']
    env['LINKCOM'] = '$LINK -o $TARGET $LINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS -Wl,--start-group $_LIBFLAGS -Wl,--end-group'
    env['ARCH'] = ArchConfig.ARCH
    env['ARCH_CPU'] = ArchConfig.ARCH_CPU
    env['ArchConfig'] = ArchConfig
    env['UserRoot'] = USR_ROOT
    # patch for win32 spawn
    if env['PLATFORM'] == 'win32':
        win32_spawn = Win32Spawn()
        win32_spawn.env = env
        env['SPAWN'] = win32_spawn.spawn

    TOOL_INSTALL(env)
    SetEnv(env)

    if not GetOption('verbose'):
        # override the default verbose command string
        env.Replace(
            ARCOMSTR = 'AR $TARGET',
            ASCOMSTR = 'AS $TARGET',
            ASPPCOMSTR = 'AS $TARGET',
            CCCOMSTR = 'CC $TARGET',
            SHCCCOMSTR = 'CC $TARGET',
            CXXCOMSTR = 'CXX $TARGET',
            SHCXXCOMSTR = 'CXX $TARGET',
            RANLIBCOMSTR = 'RANLIB $TARGET',
            LINKCOMSTR = 'LINK $TARGET',
            SHLINKCOMSTR = 'LINK $TARGET'
        )

    if not GetOption('release'):
        # do debug building
        env['ASFLAGS']  += ArchConfig.BUILD_DEBUG
        env['CCFLAGS'] += ArchConfig.BUILD_DEBUG
    else:
        # do release building
        env['ASFLAGS']  += ArchConfig.BUILD_RELEASE
        env['CCFLAGS'] += ArchConfig.BUILD_RELEASE

    return env

def GetCurrentDir():
    sconscript = File('SConscript')
    fn = sconscript.rfile()
    name = fn.name
    path = os.path.dirname(fn.abspath)
    return path

# SCons PreProcessor patch
def start_handling_includes(self, t=None):
    """
    Causes the PreProcessor object to start processing #import,
    #include and #include_next lines.

    This method will be called when a #if, #ifdef, #ifndef or #elif
    evaluates True, or when we reach the #else in a #if, #ifdef,
    #ifndef or #elif block where a condition already evaluated
    False.

    """
    d = self.dispatch_table
    p = self.stack[-1] if self.stack else self.default_table

    for k in ('import', 'include', 'include_next', 'define'):
        d[k] = p[k]

def stop_handling_includes(self, t=None):
    """
    Causes the PreProcessor object to stop processing #import,
    #include and #include_next lines.

    This method will be called when a #if, #ifdef, #ifndef or #elif
    evaluates False, or when we reach the #else in a #if, #ifdef,
    #ifndef or #elif block where a condition already evaluated True.
    """
    d = self.dispatch_table
    d['import'] = self.do_nothing
    d['include'] =  self.do_nothing
    d['include_next'] =  self.do_nothing
    d['define'] =  self.do_nothing

PatchedPreProcessor = SCons.cpp.PreProcessor
PatchedPreProcessor.start_handling_includes = start_handling_includes
PatchedPreProcessor.stop_handling_includes = stop_handling_includes

def GetDepend(depend):
    if depend == '':
        return True

    building = True
    if type(depend) == type('str'):
        building = BuildOptions.get(depend, False)
        if building == False:
            building = DependBuildOptions.get(depend, False)

        if building == '':
            building = True
        return building

    # for list type depend
    for item in depend:
        if item == '':
            return True

        building = BuildOptions.get(item, False)
        if building == False:
            building = DependBuildOptions.get(item, False)

        if building == False:
            break

    if building == '':
        building = True
    return building

def AddDepend(option):
    global DependBuildOptions
    DependBuildOptions[option] = 1

def GetConfigValue(name):
    assert type(name) == str, 'GetConfigValue: only string parameter is valid'
    try:
        return BuildOptions[name]
    except:
        return ''

def MergeGroup(src_group, group):
    src_group['src'] = src_group['src'] + group['src']

    def dict_append(d1, d2, key):
        if key in d2:
            if key in d1:
                d1[key] = d1[key] + d2[key]
            else:
                d1[key] = d2[key]

    dict_append(src_group, group, 'CCFLAGS')
    dict_append(src_group, group, 'CPPPATH')
    dict_append(src_group, group, 'CPPDEFINES')

    dict_append(src_group, group, 'LOCAL_CCFLAGS')
    dict_append(src_group, group, 'LOCAL_CPPPATH')
    dict_append(src_group, group, 'LOCAL_CPPDEFINES')

    dict_append(src_group, group, 'LINKFLAGS')
    dict_append(src_group, group, 'LIBS')
    dict_append(src_group, group, 'LIBPATH')

def DefineGroup(name, src, depend, **parameters):
    global Env

    if not GetDepend(depend):
        return []

    # find exist group and get path of group
    group_path = ''
    for item in Projects:
        if item['name'] == name:
            group_path = item['path']
    if group_path == '':
        group_path = GetCurrentDir()

    group = parameters
    group['name'] = name
    group['path'] = group_path
    if type(src) == type(['src']):
        group['src'] = File(src)
    else:
        group['src'] = src

    if 'CCFLAGS' in group:
        Env.Append(CCFLAGS = group['CCFLAGS'])
    if 'CPPPATH' in group:
        Env.AppendUnique(CPPPATH = group['CPPPATH'])
    if 'CPPDEFINES' in group:
        Env.AppendUnique(CPPDEFINES = group['CPPDEFINES'])
    if 'LINKFLAGS' in group:
        Env.Append(LINKFLAGS = group['LINKFLAGS'])

    if 'LIBS' in group:
        Env.Append(LIBS = group['LIBS'])
    if 'LIBPATH' in group:
        Env.AppendUnique(LIBPATH = group['LIBPATH'])

    if 'LIBRARY' in group:
        objs = Env.Library(name, group['src'])
    else:
        objs = group['src']

    if 'LOCAL_CPPPATH' in group:
        paths = []
        for item in group['LOCAL_CPPPATH']:
            paths.append(os.path.abspath(item))
        group['LOCAL_CPPPATH'] = paths

    # merge group
    for item in Projects:
        if item['name'] == name:
            # merge to this group
            MergeGroup(item, group)
            return objs

    # add a new group
    Projects.append(group)

    return objs

def BuildConfiguration(Toolchain = None, CPU_Arch = None):
    global ArchConfig
    global USR_ROOT

    EXEC_PATH = None
    PREFIX    = None

    if USR_ROOT and Toolchain == None and CPU_Arch == None:
        from config import ParseConfig

        config = ParseConfig(os.path.join(USR_ROOT, '.config'))

        CPU_Arch  = config.get('ARCH', 'Cortex-A')
        Toolchain = config.get('Toolchain', 'gcc')
        EXEC_PATH = config.get('EXEC_PATH', '/usr/bin')
        PREFIX = config.get('PREFIX', 'arm-none-eabi-')

    if Toolchain == 'gcc_musl':
        global GCC_MUSL
        GCC_MUSL = True

    # get EXEC_PATH from environment
    if os.getenv('RTT_EXEC_PATH'):
        EXEC_PATH = os.getenv('RTT_EXEC_PATH')

    # Build Configuration for Toolchain, CPU/Arch
    if CPU_Arch == 'Cortex-A':
        from cortexa import ARCHCortexA
        ArchConfig = ARCHCortexA(Toolchain = Toolchain, CPU_Arch = CPU_Arch, EXEC_PATH = EXEC_PATH, USR_ROOT = USR_ROOT, PREFIX = PREFIX)
    elif CPU_Arch == 'AArch64':
        from aarch64 import ARCHAARCH64
        ArchConfig = ARCHAARCH64(Toolchain = Toolchain, CPU_Arch = CPU_Arch, EXEC_PATH = EXEC_PATH, USR_ROOT = USR_ROOT, PREFIX = PREFIX)
    elif CPU_Arch == 'RISCV64':
        from riscv64 import ARCHRISCV64
        ArchConfig = ARCHRISCV64(Toolchain = Toolchain, CPU_Arch = CPU_Arch, EXEC_PATH = EXEC_PATH, USR_ROOT = USR_ROOT, PREFIX = PREFIX)
    elif CPU_Arch == 'I386':
        from i386 import ARCHI386
        ArchConfig = ARCHI386(Toolchain = Toolchain, CPU_Arch = CPU_Arch, EXEC_PATH = EXEC_PATH, USR_ROOT = USR_ROOT, PREFIX = PREFIX)

    from gcc import GenerateMUSLConfig
    GenerateMUSLConfig(ArchConfig)

    return

def ParseOptions(filename):
    PreProcessor = PatchedPreProcessor()
    contents = ''
    try:
        contents = open(filename, 'r').read()
    except:
        print('No ' + filename + ' found.')
        exit(-1)
    PreProcessor.process_contents(contents)

    options = PreProcessor.cpp_namespace

    return options

def LinkBridge():
    cwd = GetCurrentDir()
    objs = []
    list = os.listdir(cwd)

    for item in list:
        path = os.path.join(cwd, item)
        if os.path.isfile(os.path.join(path, 'SConscript')):
            objs = objs + SConscript(os.path.join(path, 'SConscript'))

    return objs

def LocateConfiguration(usr_root):
    cwd = os.getcwd()

    if usr_root != None:
        if os.path.isfile(os.path.join(usr_root, ".config")) and os.path.isfile(os.path.join(usr_root, "rtconfig.h")):
            return usr_root

    while True:
        if os.path.isfile(os.path.join(cwd, ".config")) and os.path.isfile(os.path.join(cwd, "rtconfig.h")):
            return os.path.abspath(cwd)

        parent_dir = os.path.abspath(os.path.join(cwd, '..'))
        if parent_dir == cwd:
            break
        else:
            cwd = parent_dir

    return None

def BuildRTThread(TARGET, env = None):
    global BuildOptions

    # parse rtconfig.h to build BuildOptions
    PreProcessor = PatchedPreProcessor()

    contents = ''
    try:
        contents = open(os.path.join(USR_ROOT, 'rtconfig.h'), 'r').read()
    except e:
        print('No rtconfig.h found.')
        exit(-1)
    PreProcessor.process_contents(contents)
    BuildOptions = PreProcessor.cpp_namespace

    return

def HandleLocalGroup(env, objs):
    global Projects

    # merge all objects into one list
    def one_list(l):
        lst = []
        for item in l:
            if type(item) == type([]):
                lst += one_list(item)
            else:
                lst.append(item)
        return lst

    # handle local group
    def local_group(env, group, objects):
        if 'LOCAL_CCFLAGS' in group or 'LOCAL_CPPPATH' in group or 'LOCAL_CPPDEFINES' in group or 'LOCAL_ASFLAGS' in group:
            CCFLAGS = env.get('CCFLAGS', '') + group.get('LOCAL_CCFLAGS', '')
            CPPPATH = env.get('CPPPATH', ['']) + group.get('LOCAL_CPPPATH', [''])
            CPPDEFINES = env.get('CPPDEFINES', ['']) + group.get('LOCAL_CPPDEFINES', [''])
            ASFLAGS = env.get('ASFLAGS', '') + group.get('LOCAL_ASFLAGS', '')

            for source in group['src']:
                objects.append(env.Object(source, CCFLAGS = CCFLAGS, ASFLAGS = ASFLAGS,
                    CPPPATH = CPPPATH, CPPDEFINES = CPPDEFINES))

            return True

        return False

    objs = one_list(objs)

    # remove source files with local flags setting
    for group in Projects:
        if 'LOCAL_CCFLAGS' in group or 'LOCAL_CPPPATH' in group or 'LOCAL_CPPDEFINES' in group:
            for source in group['src']:
                for obj in objs:
                    if source.abspath == obj.abspath or (len(obj.sources) > 0 and source.abspath == obj.sources[0].abspath):
                        objs.remove(obj)

    # re-add the source files to the objects
    for group in Projects:
        local_group(env, group, objs)

    return objs

def BuildApplication(TARGET, SConscriptFile, usr_root = None, **parameters):
    global USR_ROOT
    global BuildOptions

    PrepareEnv(usr_root)

    env = BuildEnv()
    env.AppendUnique(CPPPATH = USR_ROOT)
    # use staitc flags
    env.AppendUnique(CCFLAGS   = ArchConfig.STATIC_FLAGS)
    env.AppendUnique(LINKFLAGS = ArchConfig.STATIC_FLAGS)

    # parse rtconfig.h file
    BuildOptions = ParseOptions(os.path.join(USR_ROOT, 'rtconfig.h'))

    if os.path.isfile(os.path.join(os.path.dirname(SConscriptFile), 'pkg_config.h')):
        env.AppendUnique(CPPDEFINES = ['HAVE_PKG_CONFIG_H'])
        BuildOptions.update(ParseOptions(os.path.join(os.path.dirname(SConscriptFile), 'pkg_config.h')))

    if GCC_MUSL:
        BuildOptions['RT_USING_GCC_MUSL'] = True

    if USR_ROOT == os.getcwd():
        target_vdir = 'build'
    else:
        target_vdir = 'build/' + TARGET
    objs = SConscript(SConscriptFile, variant_dir=target_vdir, duplicate=0)

    # include crt/libraries
    objs.extend(SConscript(USR_ROOT + '/sdk/SConscript', variant_dir=target_vdir + '/sdk', duplicate=0))

    # handle local group
    objs = HandleLocalGroup(env, objs)

    # build program
    if TARGET.find('.') == -1:
       TARGET = TARGET + '.' + ArchConfig.TARGET_EXT

    if os.path.abspath(USR_ROOT) == os.getcwd():
        TARGET = os.path.abspath(os.path.join(USR_ROOT, prefix, TARGET))

    target = env.Program(TARGET, objs)

    if GetOption('target'):
        target_name = GetOption('target')
        if target_name == 'vsc':
            from vsc import GenerateVSCode

            SetOption('no_exec', 1)
            env['ArchConfig'] = ArchConfig

            GenerateVSCode(env)

        exit(0)

    return target

def BuildSharedApplication(TARGET, SConscriptFile, usr_root = None, **parameters):
    global USR_ROOT
    global BuildOptions

    PrepareEnv(usr_root)

    env = BuildEnv()
    env.AppendUnique(CPPPATH = USR_ROOT)
    # use PIC flags
    env.AppendUnique(CCFLAGS   = ArchConfig.PIC_FLAGS)
    env.AppendUnique(LINKFLAGS = ArchConfig.PIC_FLAGS)

    # use shared link
    env['LINKFLAGS'] = ArchConfig.SH_LFLAGS

    # parse rtconfig.h file
    BuildOptions = ParseOptions(os.path.join(USR_ROOT, 'rtconfig.h'))

    if os.path.isfile(os.path.join(os.path.dirname(SConscriptFile), 'pkg_config.h')):
        env.AppendUnique(CPPDEFINES = ['HAVE_PKG_CONFIG_H'])
        BuildOptions.update(ParseOptions(os.path.join(os.path.dirname(SConscriptFile), 'pkg_config.h')))

    if GCC_MUSL:
        BuildOptions['RT_USING_GCC_MUSL'] = True

    if USR_ROOT == os.getcwd():
        target_vdir = 'build'
    else:
        target_vdir = 'build/' + TARGET
    objs = SConscript(SConscriptFile, variant_dir=target_vdir, duplicate=0)

    # include crt/libraries
    objs.extend(SConscript(USR_ROOT + '/sdk/SConscript', variant_dir=target_vdir + '/sdk', duplicate=0))

    # handle local group
    objs = HandleLocalGroup(env, objs)

    # build program
    if TARGET.find('.') == -1:
        TARGET = TARGET + '.' + ArchConfig.TARGET_EXT

    if os.path.abspath(USR_ROOT) == os.getcwd():
        TARGET = os.path.abspath(os.path.join(USR_ROOT, prefix, TARGET))

    target = env.Program(TARGET, objs)

    return target

def LinkFilesAction(env, target, source):
    link_file = open(str(target[0]), 'w')

    for item in source:
        item = str(item).replace('\\', '/')
        link_file.write('%s\n' % item)
    link_file.close()

    return None

def BuildLibrary(TARGET, SConscriptFile, usr_root = None, **parameters):
    global USR_ROOT

    PrepareEnv(usr_root)

    env = BuildEnv()
    env.AppendUnique(CPPPATH = USR_ROOT)
    # skip warning
    env['CCFLAGS'] = env['CCFLAGS'].replace('-Werror -Wall', '')
    if 'CCFLAGS' in parameters:
        env.AppendUnique(CCFLAGS = parameters['CCFLAGS'])
    if 'PIC' in parameters and parameters['PIC']:
        env.AppendUnique(CCFLAGS = ArchConfig.PIC_FLAGS)
    if 'CPPDEFINES' in parameters:
        env.AppendUnique(CPPDEFINES = parameters['CPPDEFINES'])
    if 'CPPPATH' in parameters:
        env.AppendUnique(CPPPATH = parameters['CPPPATH'])

    if 'build' in parameters:
        target_vdir = os.path.join('build', parameters['build'])
    else:
        target_vdir = 'build'
    objs = SConscript(SConscriptFile, variant_dir=target_vdir, duplicate=0)
    if parameters.get('nosdk', 0) == 0:
        # include crt/libraries
        objs.extend(SConscript(USR_ROOT + '/sdk/SConscript', variant_dir=target_vdir + '/sdk', duplicate=0))

    if len(objs) > 100:
        env['ARCOM'] = '$AR $ARFLAGS $TARGET @$SOURCES'

        # to object files from source files
        objs = env.Object(objs)

        builder = Builder(action = LinkFilesAction, )
        env.Append(BUILDERS = {"LinkFiles" : builder})
        link_file = env.LinkFiles(target = os.path.join(target_vdir, 'link' + TARGET + '.txt'), source = objs)

        # build library
        target = env.Library(os.path.join(target_vdir, TARGET), link_file)
    else:
        # build library
        target = env.Library(os.path.join(target_vdir, TARGET), objs)

    return target

def BuildSharedLibrary(TARGET, SConscriptFile, usr_root = None, **parameters):
    global USR_ROOT
    PrepareEnv(usr_root)

    env = BuildEnv()
    env.AppendUnique(CPPPATH = USR_ROOT)
    env.AppendUnique(CCFLAGS = ArchConfig.PIC_FLAGS)

    if 'CCFLAGS' in parameters:
        env.AppendUnique(CCFLAGS = parameters['CCFLAGS'])
    if 'PIC' in parameters and parameters['PIC']:
        env.AppendUnique(CCFLAGS = ArchConfig.PIC_FLAGS)
    if 'CPPDEFINES' in parameters:
        env.AppendUnique(CPPDEFINES = parameters['CPPDEFINES'])
    if 'CPPPATH' in parameters:
        env.AppendUnique(CPPPATH = parameters['CPPPATH'])

    if 'build' in parameters:
        target_vdir = os.path.join('build', parameters['build'])
    else:
        target_vdir = 'build'

    # empty link flags
    env['LINKFLAGS'] = ''

    objs = SConscript(SConscriptFile, variant_dir=target_vdir, duplicate=0)
    if parameters.get('nosdk', 0) == 0:
        # include crt/libraries
        objs.extend(SConscript(USR_ROOT + '/sdk/SConscript', variant_dir=target_vdir + '/sdk', duplicate=0))

    if len(objs) > 100:
        env['ARCOM'] = '$AR $ARFLAGS $TARGET @$SOURCES'

        # to object files from source files
        objs = env.Object(objs)

        builder = Builder(action = LinkFilesAction, )
        env.Append(BUILDERS = {"LinkFiles" : builder})
        link_file = env.LinkFiles(target = os.path.join(target_vdir, 'link' + TARGET + '.txt'), source = objs)

        # build library
        target = env.SharedLibrary(os.path.join(target_vdir, TARGET), link_file)
    else:
        # build dll library
        target = env.SharedLibrary(os.path.join(target_vdir, TARGET), objs)

    return target

def SrcRemove(src, remove):
    if not src:
        return

    src_bak = src[:]

    if type(remove) == type('str'):
        if os.path.isabs(remove):
            remove = os.path.relpath(remove, GetCurrentDir())
        remove = os.path.normpath(remove)

        for item in src_bak:
            if type(item) == type('str'):
                item_str = item
            else:
                item_str = item.rstr()

            if os.path.isabs(item_str):
                item_str = os.path.relpath(item_str, GetCurrentDir())
            item_str = os.path.normpath(item_str)

            if item_str == remove:
                src.remove(item)
    else:
        for remove_item in remove:
            remove_str = str(remove_item)
            if os.path.isabs(remove_str):
                remove_str = os.path.relpath(remove_str, GetCurrentDir())
            remove_str = os.path.normpath(remove_str)

            for item in src_bak:
                if type(item) == type('str'):
                    item_str = item
                else:
                    item_str = item.rstr()

                if os.path.isabs(item_str):
                    item_str = os.path.relpath(item_str, GetCurrentDir())
                item_str = os.path.normpath(item_str)

                if item_str == remove_str:
                    src.remove(item)

def LaunchMenuconfig():
    AddOption('--menuconfig',
                        dest = 'menuconfig',
                        action = 'store_true',
                        default = False,
                        help = 'make menuconfig for RT-Thread BSP')
    if GetOption('menuconfig'):
        from menuconfig import menuconfig
        menuconfig('../')
        exit(0)

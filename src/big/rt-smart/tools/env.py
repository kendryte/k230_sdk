#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

def GetHome():
    return os.environ['HOME']

def GetEnvHome():
    home = GetHome()

    env_home = os.path.join(home, '.env')

    if not os.path.exists(env_home):
        os.mkdir(env_home)

    return env_home

def Build():
    '''
    Build Target
    '''
    try:
        import sys

        scons_path = os.path.join(sys.prefix, 'lib', 'scons')
        if not os.path.isdir(scons_path):
            scons_path = os.path.join(sys.prefix, 'Lib', 'site-packages', 'scons')

        if not os.path.isdir(scons_path):
            for item in sys.path:
                if os.path.isdir(os.path.join(item, 'scons')):
                    scons_path = os.path.join(item, 'scons')
                    break

        # print(scons_path)

        sys.path = sys.path + [scons_path, os.path.dirname(__file__)]

        import SCons.Script

        SCons.Script.main()
    except Exception as e:
       print('building with scons failed!')
       print(e)

    return

def Config(userapps_root):
    sys.path = sys.path + [os.path.dirname(__file__)]

    import menuconfig
    menuconfig.menuconfig()

    return

def Env():
    if len(sys.argv) > 1:
        if sys.argv[1] == 'menuconfig':

            userapps_root = os.path.abspath(os.getcwd())
            print("User APP path => %s" % userapps_root)
            if len(sys.argv) == 3:
                if os.path.isfile(os.path.join('apps', sys.argv[2], 'Uconfig')):
                    os.chdir(os.path.join('apps', sys.argv[2]))
                    Config(userapps_root)
                else:
                    print("No Uconfig under %s" % sys.argv[2])
            else:
                userapps_root = None
                if os.path.isfile('Uconfig'):
                    Config(userapps_root)
                else:
                    print("No Uconfig file")
            return

    Build()

if __name__ == '__main__':
    Build()

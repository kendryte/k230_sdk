#!/usr/bin/env python
# encoding: utf-8

import sys
import pexpect
import time
import os

spawn_cmd = 'sudo minicom -D /dev/ttyUSB5 -b 115200'
encoding = 'utf-8'
settingencoding = encoding

err_list = ['Operation not permitted', 'Resource busy', 'getcwd', 'error']
global test_result
global test_pass_index
test_result = ['{Test PASSED}', '(?i)error', '(?i)fail', '(?i)assert', pexpect.TIMEOUT]
test_pass_index = 0
def BoxStartup():
    time.sleep(1)
    child = pexpect.spawn(spawn_cmd, timeout=30, echo=False, encoding='utf-8')
    logFileId = open("detaillog.txt", 'a+')
    child.logfile_read = logFileId
    # child.logfile = sys.stdout
    index = child.expect(['ubuntu',pexpect.TIMEOUT],timeout=1)
    if index == 0:
        child.send('canaan\n')
    else :
        child.send('\n')
    # child.logfile_read = sys.stdout
    # child.expect_exact(' \ | /')
    qemu_output = child.before
    child.send('\n')
    child.expect('msh')
    return child

def BoxComDir(box, name):
    ret = 0
    box.sendline('ls /sdcard/')
    ret = box.expect([name, pexpect.TIMEOUT],timeout=1)
    if ret == 0:
        box.sendline('cd /sdcard/')
        box.expect('sdcard')
        return 1
    else:
        print('cannot find testcases in /sdcard:'+name)

    box.sendline('ls /bin/')
    ret = box.expect([name, pexpect.TIMEOUT],timeout=1)
    if ret == 0:
        box.sendline('cd /bin/')
        box.expect('bin')
        return 0
    else:
        print('cannot find testcases /bin:'+name)
    return -1




def BoxDelay(box, second):
    import time
    time.sleep(second)

def BoxReady(box):
    box.send('\n')
    index = box.expect(['msh', pexpect.TIMEOUT])
    if index == 0:
        return True

    return False

def BoxFileSystemReady(box):
    box.send('ls mnt\n')
    result = box.expect(['Directory mnt:', 'No such directory'])

    if result == 1:
        return False

    return True

def BoxFileSystemFormat(box):
    print('do file system format...')

    box.sendline('mkfs sd0')
    ret = box.expect(['mkfs failed', pexpect.TIMEOUT], timeout=1)

    if ret == 0:
        print('Format file system failed!')
        return False

    return True

def BoxReload(box):
    BoxClose(box)
    return BoxStartup()


def _BoxRunCmd(box, cmd, expect=None, testtimeout=2):
    box.sendline(cmd)
    if expect:
        box.timeout = testtimeout
        result = box.expect(expect)
        print(box.before)
        print(box.after)
        return result

    return 0

def BoxRunCmd(box, cmd, expect=None, testtimeout=2):
    real_expect = []
    isExpect = 0
    if not expect:
        return 0
    if len(expect) == 2:
        isExpect = 1
    if isExpect:
        real_expect.append(expect[0])
        for i in range(0, len(err_list)):
            real_expect.append(err_list[i])
        real_expect.append(expect[1])
        tmp_result = _BoxRunCmd(box, cmd, real_expect, testtimeout)
        if tmp_result == 0:
            return 0
        else:
            return 1
    else:
        for i in range(0, len(err_list)):
            real_expect.append(err_list[i])
        real_expect.append(expect[0])
        tmp_result = _BoxRunCmd(box, cmd, real_expect, testtimeout)
        if tmp_result == len(err_list):
            return 0
        else:
            return 1

def BoxLogClear(box):
    before = box.before
    after = box.after

def BoxClose(box):
    try:
        box.sendcontrol('a')
        box.send('x')
        box.send('\n')
        box.close(force=True)
    except:
        os.system("pkill term")

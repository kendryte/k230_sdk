#!/usr/bin/env python
# encoding: utf-8

import pexpect
import qemubox  
import time
import sys
import os

box = ''
passed = 0
dirlist = ['time_tc/gmtime/1-1',
'time_tc/gmtime/2-1',
'time_tc/time/1-1',
'time_tc/clock_settime/6-1',
'time_tc/difftime/1-1',
'time_tc/clock_getres/3-1',
'time_tc/clock_getres/1-1',
'time_tc/clock_getres/7-1',
'time_tc/localtime_r/1-1',
'time_tc/asctime/1-1',
'time_tc/asctime_r/1-1',
'time_tc/gmtime_r/1-1',
'time_tc/localtime/1-1',
'time_tc/clock/2-1',
'time_tc/mktime/1-1',
'time_tc/ctime_r/1-1',
'time_tc/clock_gettime/7-1',
'time_tc/ctime/1-1',]
test_name = 'time_tc'

def do_test(cmd):
    global box
    global passed
    global dirlist

    qemubox.BoxReady(box)

    box.sendline('ls')
    box.expect('DIR')

    box.sendline(cmd)
    index = box.expect(qemubox.test_result, timeout = 20)
    if index <= qemubox.test_pass_index :
        print(cmd + '...Test PASSED')
        passed = passed + 1
    else:
        print(cmd + '...FAILED')


if __name__=='__main__':
    # l = os.walk('../bin')

    cmds = []

    # for (dirname, sdir, sfile) in l:
    #     for item in sfile:
    #         cmd = dirname + '/' + item
    #         cmds.append(cmd[7:])
    #         print('\''+cmd[7:]+'\',')
    
    for ldir in dirlist:
        cmds.append(ldir)
        print(ldir)

    box = qemubox.BoxStartup()
    ret = qemubox.BoxComDir(box, test_name)
    if ret == 0:
        print('cd to /bin')
    elif ret == 1:
        print('cd to /sdcard')
    else:
        qemubox.BoxClose(box)
        sys.exit()
    for item in cmds:
        do_test(item)
    qemubox.BoxClose(box)

    print('Total Testcases: %d' % len(cmds))
    print('         Passed: %d' % passed)
    print('    Pass Rate: %.2f' % (passed / len(cmds)))

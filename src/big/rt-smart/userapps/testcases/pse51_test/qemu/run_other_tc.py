#!/usr/bin/env python
# encoding: utf-8

import pexpect
import qemubox  
import time
import sys
import os

box = ''
passed = 0
dirlist = ['other_tc/fcntl_h/open_tc',
'other_tc/lio_listio/6-1',
'other_tc/errno_h/errno_tc',
'other_tc/fork/3-1',
'other_tc/fork/2-1',
'other_tc/fsync/4-1',
'other_tc/fsync/5-1',]
test_name = 'other_tc'

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

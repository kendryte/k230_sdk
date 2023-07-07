#!/usr/bin/env python
# encoding: utf-8

import pexpect
import qemubox  
import time
import sys
import os

box = ''
passed = 0
dirlist = ['signal_tc/signal/6-1',
'signal_tc/signal/5-1',
'signal_tc/sigaction/2-9',
'signal_tc/sigaction/2-25',
'signal_tc/sigaction/2-18',
'signal_tc/sigaction/2-17',
'signal_tc/sigaction/2-23',
'signal_tc/sigaction/2-20',
'signal_tc/sigaction/2-8',
'signal_tc/sigaction/2-22',
'signal_tc/sigaction/2-14',
'signal_tc/sigaction/2-15',
'signal_tc/sigaction/2-2',
'signal_tc/sigaction/2-10',
'signal_tc/sigaction/2-5',
'signal_tc/sigaction/2-1',
'signal_tc/sigaction/2-4',
'signal_tc/sigaction/2-16',
'signal_tc/sigfillset/1-1',
'signal_tc/sigfillset/2-1',
'signal_tc/sigemptyset/1-1',
'signal_tc/sigemptyset/2-1',
'signal_tc/sigdelset/4-1',
'signal_tc/sigdelset/1-3',
'signal_tc/sigdelset/1-1',
'signal_tc/sigdelset/1-2',
'signal_tc/sigdelset/1-4',
'signal_tc/sigaddset/4-1',
'signal_tc/sigaddset/1-3',
'signal_tc/sigaddset/1-1',
'signal_tc/sigaddset/1-2',
'signal_tc/sigaddset/2-1',
'signal_tc/sigprocmask/12-1',
'signal_tc/sigprocmask/17-1',
'signal_tc/sigprocmask/8-1',
'signal_tc/sigprocmask/8-3',
'signal_tc/sigprocmask/15-1',
'signal_tc/sigprocmask/7-1',
'signal_tc/sigprocmask/8-2',
'signal_tc/sigprocmask/10-1',
'signal_tc/raise/6-1',
'signal_tc/raise/4-1',
'signal_tc/raise/1-1',
'signal_tc/sigqueue/9-1',
'signal_tc/sigqueue/7-1',
'signal_tc/sigqueue/2-1',
'signal_tc/killpg/6-1',
'signal_tc/killpg/8-1',
'signal_tc/killpg/5-1',
'signal_tc/sigismember/4-1',
'signal_tc/sigismember/3-1',
'signal_tc/kill/1-1',
'signal_tc/kill/2-1',]
test_name = 'signal_tc'

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

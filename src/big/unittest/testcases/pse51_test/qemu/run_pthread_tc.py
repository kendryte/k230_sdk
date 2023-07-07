#!/usr/bin/env python
# encoding: utf-8

import pexpect
import qemubox  
import time
import sys
import os

box = ''
passed = 0
dirlist = ['pthread_tc/pthread_sigmask/12-1',
'pthread_tc/pthread_sigmask/16-1',
'pthread_tc/pthread_sigmask/14-1',
'pthread_tc/pthread_sigmask/8-1',
'pthread_tc/pthread_sigmask/8-3',
'pthread_tc/pthread_sigmask/15-1',
'pthread_tc/pthread_sigmask/7-1',
'pthread_tc/pthread_sigmask/8-2',
'pthread_tc/pthread_sigmask/10-1',
'pthread_tc/pthread_mutex_unlock/3-1',
'pthread_tc/pthread_mutex_unlock/1-1',
'pthread_tc/pthread_mutex_unlock/2-1',
'pthread_tc/pthread_condattr_init/3-1',
'pthread_tc/pthread_condattr_init/1-1',
'pthread_tc/pthread_self/1-1',
'pthread_tc/pthread_cleanup_push/1-3',
'pthread_tc/pthread_cleanup_push/1-1',
'pthread_tc/pthread_cleanup_pop/1-3',
'pthread_tc/pthread_cleanup_pop/1-1',
'pthread_tc/pthread_cleanup_pop/1-2',
'pthread_tc/pthread_condattr_setpshared/1-1',
'pthread_tc/pthread_condattr_setpshared/1-2',
'pthread_tc/pthread_condattr_setpshared/2-1',
'pthread_tc/pthread_join/1-1',
'pthread_tc/pthread_join/5-1',
'pthread_tc/pthread_join/2-1',
'pthread_tc/pthread_cond_destroy/3-1',
'pthread_tc/pthread_cond_destroy/1-1',
'pthread_tc/pthread_equal/1-1',
'pthread_tc/pthread_attr_getscope/1-1',
'pthread_tc/pthread_condattr_setclock/1-1',
'pthread_tc/pthread_condattr_setclock/1-2',
'pthread_tc/pthread_condattr_setclock/2-1',
'pthread_tc/pthread_detach/3-1',
'pthread_tc/pthread_detach/2-1',
'pthread_tc/pthread_key_delete/1-1',
'pthread_tc/pthread_key_delete/1-2',
'pthread_tc/pthread_key_delete/2-1',
'pthread_tc/pthread_attr_setstacksize/1-1',
'pthread_tc/pthread_condattr_getclock/1-1',
'pthread_tc/pthread_condattr_getclock/1-2',
'pthread_tc/pthread_mutexattr_init/3-1',
'pthread_tc/pthread_mutexattr_init/1-1',
'pthread_tc/pthread_exit/3-1',
'pthread_tc/pthread_exit/1-1',
'pthread_tc/pthread_exit/2-1',
'pthread_tc/pthread_setspecific/1-1',
'pthread_tc/pthread_setspecific/1-2',
'pthread_tc/pthread_attr_getschedpolicy/2-1',
'pthread_tc/pthread_mutex_lock/2-1',
'pthread_tc/pthread_attr_setdetachstate/1-1',
'pthread_tc/pthread_attr_setdetachstate/1-2',
'pthread_tc/pthread_mutexattr_setprotocol/3-2',
'pthread_tc/pthread_mutexattr_setprotocol/3-1',
'pthread_tc/pthread_setschedparam/1-1',
'pthread_tc/pthread_condattr_getpshared/1-1',
'pthread_tc/pthread_condattr_getpshared/1-2',
'pthread_tc/pthread_condattr_getpshared/2-1',
'pthread_tc/pthread_mutex_trylock/4-1',
'pthread_tc/pthread_mutex_trylock/3-1',
'pthread_tc/pthread_create/1-1',
'pthread_tc/pthread_cond_broadcast/2-2',
'pthread_tc/pthread_attr_getstacksize/1-1',
'pthread_tc/pthread_cond_signal/4-1',
'pthread_tc/pthread_cond_signal/1-1',
'pthread_tc/pthread_cancel/2-3',
'pthread_tc/pthread_cancel/4-1',
'pthread_tc/pthread_cancel/3-1',
'pthread_tc/pthread_cancel/1-1',
'pthread_tc/pthread_cancel/2-2',
'pthread_tc/pthread_cancel/1-2',
'pthread_tc/pthread_cancel/2-1',
'pthread_tc/pthread_attr_setschedpolicy/5-1',
'pthread_tc/pthread_mutexattr_getpshared/1-3',
'pthread_tc/pthread_mutexattr_getpshared/3-1',
'pthread_tc/pthread_mutexattr_getpshared/1-1',
'pthread_tc/pthread_mutexattr_getpshared/1-2',
'pthread_tc/pthread_attr_destroy/1-1',
'pthread_tc/pthread_atfork/1-1',
'pthread_tc/pthread_atfork/2-1',
'pthread_tc/pthread_cond_init/1-1',
'pthread_tc/pthread_cond_wait/3-1',
'pthread_tc/pthread_mutex_destroy/3-1',
'pthread_tc/pthread_mutex_destroy/5-1',
'pthread_tc/pthread_mutex_destroy/2-2',
'pthread_tc/pthread_mutex_destroy/2-1',
'pthread_tc/pthread_mutex_destroy/5-2',
'pthread_tc/pthread_kill/1-1',
'pthread_tc/pthread_testcancel/2-1',
'pthread_tc/pthread_attr_setinheritsched/4-1',
'pthread_tc/pthread_attr_setinheritsched/1-1',
'pthread_tc/pthread_mutexattr_getprotocol/1-1',
'pthread_tc/pthread_key_create/3-1',
'pthread_tc/pthread_key_create/1-1',
'pthread_tc/pthread_key_create/1-2',
'pthread_tc/pthread_key_create/2-1',
'pthread_tc/pthread_attr_getschedparam/1-1',
'pthread_tc/pthread_once/1-1',
'pthread_tc/pthread_condattr_destroy/1-1',
'pthread_tc/pthread_condattr_destroy/2-1',
'pthread_tc/pthread_setcancelstate/3-1',
'pthread_tc/pthread_cond_timedwait/3-1',
'pthread_tc/pthread_cond_timedwait/1-1',
'pthread_tc/pthread_cond_timedwait/2-2',
'pthread_tc/pthread_cond_timedwait/2-1',
'pthread_tc/pthread_mutexattr_settype/1-1',
'pthread_tc/pthread_attr_init/1-1',
'pthread_tc/pthread_mutexattr_gettype/1-3',
'pthread_tc/pthread_mutexattr_gettype/1-1',
'pthread_tc/pthread_mutexattr_gettype/1-2',
'pthread_tc/pthread_mutexattr_gettype/1-5',
'pthread_tc/pthread_mutexattr_gettype/1-4',
'pthread_tc/pthread_mutex_init/1-1',
'pthread_tc/pthread_attr_getdetachstate/1-1',
'pthread_tc/pthread_attr_setschedparam/1-1',
'pthread_tc/pthread_attr_setschedparam/1-2',
'pthread_tc/pthread_attr_setscope/4-1',
'pthread_tc/pthread_attr_setscope/1-1',
'pthread_tc/pthread_mutexattr_setpshared/3-2',
'pthread_tc/pthread_mutexattr_setpshared/3-1',
'pthread_tc/pthread_mutexattr_setpshared/1-1',
'pthread_tc/pthread_mutexattr_setpshared/2-2',
'pthread_tc/pthread_mutexattr_setpshared/1-2',
'pthread_tc/pthread_mutexattr_setpshared/2-1',
'pthread_tc/pthread_getspecific/3-1',
'pthread_tc/pthread_getspecific/1-1',
'pthread_tc/pthread_attr_getinheritsched/1-1',
'pthread_tc/pthread_mutexattr_destroy/1-1',
'pthread_tc/pthread_mutexattr_destroy/2-1',
'pthread_tc/pthread_getschedparam/1-1',
'pthread_tc/pthread_getschedparam/1-2',]
test_name = 'pthread_tc'

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

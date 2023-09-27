#!/usr/bin/env python
# encoding: utf-8

import pexpect
import qemubox  
import time
import sys
import os

box = ''
passed = 0
dirlist = ['str_tc/strftime/3-1',
'str_tc/strftime/1-1',
'str_tc/strftime/2-1',
'str_tc/strncpy/1-1',
'str_tc/strncpy/2-1',
'str_tc/inttypes_h/strtoumax_tc',
'str_tc/inttypes_h/strtoimax_tc',
'str_tc/strchr/1-1',
'str_tc/strcpy/1-1',
'str_tc/strlen/1-1',
'str_tc/string_h/strcspn_tc',
'str_tc/string_h/strncat_tc',
'str_tc/string_h/strxfrm_tc',
'str_tc/string_h/strpbrk_tc',
'str_tc/string_h/strspn_tc',
'str_tc/string_h/memcpy_tc',
'str_tc/string_h/memset_tc',
'str_tc/string_h/strcoll_tc',
'str_tc/string_h/strtok_r_tc',
'str_tc/string_h/strstr_tc',
'str_tc/string_h/strcat_tc',
'str_tc/string_h/strtok_tc',
'str_tc/string_h/memmove_tc',
'str_tc/string_h/strrchr_tc',]
test_name = 'str_tc'

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

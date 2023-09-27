#!/usr/bin/env python
# encoding: utf-8

import pexpect
import qemubox  
import time
import sys
import os

box = ''
passed = 0
dirlist = ['std_tc/unistd_h/getpid_tc',
'std_tc/unistd_h/ftruncate_tc',
'std_tc/unistd_h/pipe_tc',
'std_tc/unistd_h/open_read_write_fsync_close_tc',
'std_tc/unistd_h/sleep_tc',
'std_tc/unistd_h/access_tc',
'std_tc/stdio_h/getchar_tc',
'std_tc/stdio_h/opendir_tc',
'std_tc/stdio_h/stdin',
'std_tc/stdio_h/readdir_r_tc',
'std_tc/stdio_h/vfprintf',
'std_tc/stdio_h/setvbuf',
'std_tc/stdio_h/setbuf',
'std_tc/stdio_h/vsscanf',
'std_tc/stdio_h/fflush_tc',
'std_tc/stdio_h/readdir_tc',
'std_tc/stdio_h/rewinddir',
'std_tc/stdio_h/vsscanf_tc',
'std_tc/stdio_h/ferr_tc',
'std_tc/stdio_h/vsnprintf',
'std_tc/stdio_h/sscanf_tc',
'std_tc/stdio_h/closedir_tc',
'std_tc/stdio_h/fileno_tc',
'std_tc/stdio_h/gets_tc',
'std_tc/stdio_h/putchar_tc',
'std_tc/stdio_h/printf_tc',
'std_tc/stdio_h/vscanf_tc',
'std_tc/stdio_h/vfscanf',
'std_tc/stdio_h/getchar_unlocked_tc',
'std_tc/stdio_h/scanf',
'std_tc/stdio_h/vprintf_tc',
'std_tc/stdio_h/vsprintf_tc',
'std_tc/stdio_h/stderr_tc',
'std_tc/stdio_h/sprintf_tc',
'std_tc/stdio_h/stdout',
'std_tc/stdio_h/getc_unlocked_tc',
'std_tc/stdio_h/getc_tc',
'std_tc/stdlib_h/bsearch_tc',
'std_tc/stdlib_h/atoll_tc',
'std_tc/stdlib_h/strtoll_tc',
'std_tc/stdlib_h/rand_r_tc',
'std_tc/stdlib_h/setenv_tc',
'std_tc/stdlib_h/strtoul_tc',
'std_tc/stdlib_h/lldiv_tc',
'std_tc/stdlib_h/strtod_tc',
'std_tc/stdlib_h/strtol_tc',
'std_tc/stdlib_h/div_tc',
'std_tc/stdlib_h/ldiv_tc',
'std_tc/stdlib_h/calloc_tc',
'std_tc/stdlib_h/atoi_tc',
'std_tc/stdlib_h/strtof_tc',
'std_tc/stdlib_h/llabs_tc',
'std_tc/stdlib_h/atol_tc',
'std_tc/stdlib_h/atof_tc',
'std_tc/stdlib_h/labs_tc',
'std_tc/stdlib_h/mktime_tc',
'std_tc/stdlib_h/getenv_tc',
'std_tc/stdlib_h/realloc_tc',
'std_tc/stdlib_h/qsort_tc',
'std_tc/stdlib_h/rand_tc',
'std_tc/stdlib_h/unsetenv_tc',
'std_tc/stdlib_h/abs_tc',
'std_tc/stdarg_h/va_copy_tc',
'std_tc/stdarg_h/va_arg_tc',]
test_name = 'std_tc'

def do_test(cmd):
    global box
    global passed
    global dirlist

    qemubox.BoxReady(box)
    box.sendline(cmd)
    isInputTest = 0
    index = box.expect(["please input alt","please input char","please input string",pexpect.TIMEOUT],timeout=3)
    if index == 0:
        isInputTest = 1
        print('inputing long string.')
        time.sleep(0.3)
        box.send('hello 1234')
        time.sleep(0.3)
        box.send('^[\r\n')
    elif index == 1:
        isInputTest = 1
        print('inputing char.')
        time.sleep(0.3)
        box.send('a')
        time.sleep(0.3)
        box.send('\r\n')
    elif index == 2:
        isInputTest = 1
        print('inputing string.')
        time.sleep(0.3)
        box.send('hello')
        time.sleep(0.3)
        box.send('\r\n')
        time.sleep(0.3)
        box.send('1234')
        time.sleep(0.3)
        box.send('\r\n')
    index = box.expect(qemubox.test_result, timeout = 15)
    if index <= qemubox.test_pass_index :
        print(cmd + '...Test PASSED')
        passed = passed + 1
    elif index == len(qemubox.test_result)-1 and isInputTest == 1:
        while index == len(qemubox.test_result)-1:
            print('input timeout')
            box.send("^[\r\n")
            index = box.expect(qemubox.test_result, timeout = 15)
            if index <= qemubox.test_pass_index :
                print(cmd + '...Test PASSED')
                passed = passed + 1
            else:
                print(cmd + '...FAILED')
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

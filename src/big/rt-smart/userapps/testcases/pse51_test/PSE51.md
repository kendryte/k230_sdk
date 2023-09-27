# RISC-V & PSE51 测试用例

# 测试结果

详见：[腾讯在线文档](https://docs.qq.com/sheet/DVmNVS2hyU1l5cGN4)

| 不支持： | 47   | 16.61% |
| -------- | ---- | ------ |
| 支持：   | 210  | 74.20% |
| 已测试： | 257  | 90.81% |


|      C库      |                函数                |     支持情况     |                             备注                             |
| :-----------: | :--------------------------------: | :--------------: | :----------------------------------------------------------: |
|    ctype.h    |                                    |                  |                                                              |
|               |             isalnum()              | Smart-RISCV 支持 |                                                              |
|               |             isalpha()              | Smart-RISCV 支持 |                                                              |
|               |             isblank()              | Smart-RISCV 支持 |                                                              |
|               |             iscntrl()              | Smart-RISCV 支持 |                                                              |
|               |             isdigit()              | Smart-RISCV 支持 |                                                              |
|               |             isgraph()              | Smart-RISCV 支持 |                                                              |
|               |             islower()              | Smart-RISCV 支持 |                                                              |
|               |             isprint()              | Smart-RISCV 支持 |                                                              |
|               |             ispunct()              | Smart-RISCV 支持 |                                                              |
|               |             isspace()              | Smart-RISCV 支持 |                                                              |
|               |             isupper()              | Smart-RISCV 支持 |                                                              |
|               |             isxdigit()             | Smart-RISCV 支持 |                                                              |
|               |             tolower()              | Smart-RISCV 支持 |                                                              |
|               |             toupper()              | Smart-RISCV 支持 |                                                              |
|    errno.h    |                                    |                  |                                                              |
|               |               errno                | Smart-RISCV 支持 |                                                              |
|    fcntl.h    |                                    |                  |                                                              |
|               |               open()               | Smart-RISCV 支持 |                                                              |
|    fenv.h     |                                    |                  |                                                              |
|               |         sched_setscheduler         |      不支持      |                                                              |
|               |             fegetenv()             | Smart-RISCV 支持 |                                                              |
|               |         fegetexceptflag()          | Smart-RISCV 支持 |                                                              |
|               |            fegetround()            | Smart-RISCV 支持 |                                                              |
|               |           feholdexcept()           | Smart-RISCV 支持 |                                                              |
|               |          feraiseexcept()           | Smart-RISCV 支持 |                                                              |
|               |             fesetenv()             | Smart-RISCV 支持 |                                                              |
|               |         fesetexceptflag()          | Smart-RISCV 支持 |                                                              |
|               |            fesetround()            | Smart-RISCV 支持 |                                                              |
|               |           fetestexcept()           | Smart-RISCV 支持 |                                                              |
|               |           feupdateenv()            | Smart-RISCV 支持 |                                                              |
|  inttypes.h   |                                    |                  |                                                              |
|               |             imaxabs()              | Smart-RISCV 支持 |                                                              |
|               |             imaxdiv()              | Smart-RISCV 支持 |                                                              |
|               |            strtoimax()             | Smart-RISCV 支持 |                                                              |
|               |            strtoumax()             | Smart-RISCV 支持 |                                                              |
|   locale.h    |                                    |                  |                                                              |
|               |            localeconv()            |      不支持      |                                                              |
|               |            setlocale()             |      不支持      |                                                              |
|   pthread.h   |                                    |                  |                                                              |
|               |          pthread_atfork()          | Smart-RISCV 支持 |                                                              |
|               |       pthread_attr_destroy()       | Smart-RISCV 支持 |                                                              |
|               |   pthread_attr_getdetachstate()    | Smart-RISCV 支持 |                                                              |
|               |    pthread_attr_getguardsize()     |                  |                                                              |
|               |   pthread_attr_getinheritsched()   | Smart-RISCV 支持 |                                                              |
|               |    pthread_attr_getschedparam()    | Smart-RISCV 支持 |                                                              |
|               |   pthread_attr_getschedpolicy()    | Smart-RISCV 支持 |                                                              |
|               |      pthread_attr_getscope()       |      不支持      |                                                              |
|               |      pthread_attr_getstack()       |      不支持      |                                                              |
|               |    pthread_attr_getstackaddr()     |                  |                                                              |
|               |    pthread_attr_getstacksize()     | Smart-RISCV 支持 |                                                              |
|               |        pthread_attr_init()         | Smart-RISCV 支持 |                                                              |
|               |   pthread_attr_setdetachstate()    | Smart-RISCV 支持 |                                                              |
|               |    pthread_attr_setguardsize()     |                  |                                                              |
|               |   pthread_attr_setinheritsched()   | Smart-RISCV 支持 |                                                              |
|               |    pthread_attr_setschedparam()    | Smart-RISCV 支持 |                                                              |
|               |   pthread_attr_setschedpolicy()    | Smart-RISCV 支持 |                           6过1 5-1                           |
|               |      pthread_attr_setscope()       | Smart-RISCV 支持 |                                                              |
|               |      pthread_attr_setstack()       |                  |                                                              |
|               |    pthread_attr_setstackaddr()     | Smart-RISCV 支持 |                                                              |
|               |    pthread_attr_setstacksize()     | Smart-RISCV 支持 |                                                              |
|               |          pthread_cancel()          | Smart-RISCV 支持 |                        9过2 1-2、4-1                         |
|               |       pthread_cleanup_pop()        | Smart-RISCV 支持 |                                                              |
|               |       pthread_cleanup_push()       | Smart-RISCV 支持 |                                                              |
|               |      pthread_cond_broadcast()      |      不支持      |                                                              |
|               |       pthread_cond_destroy()       | Smart-RISCV 支持 |                                                              |
|               |        pthread_cond_init()         | Smart-RISCV 支持 |                                                              |
|               |       pthread_cond_signal()        | Smart-RISCV 支持 |                        6过2 1-1、4-1                         |
|               |      pthread_cond_timedwait()      |      不支持      |                                                              |
|               |        pthread_cond_wait()         | Smart-RISCV 支持 |                                                              |
|               |     pthread_condattr_destroy()     | Smart-RISCV 支持 |                                                              |
|               |    pthread_condattr_getclock()     | Smart-RISCV 支持 |                                                              |
|               |      pthread_condattr_init()       | Smart-RISCV 支持 |                                                              |
|               |    pthread_condattr_setclock()     | Smart-RISCV 支持 |                                                              |
|               |          pthread_create()          | Smart-RISCV 支持 |                                                              |
|               |          pthread_detach()          | Smart-RISCV 支持 |                        8过2 2-1、3-1                         |
|               |          pthread_equal()           | Smart-RISCV 支持 |                                                              |
|               |           pthread_exit()           |      不支持      |                     10过3 1-1、2-1、3-1                      |
|               |      pthread_getcpuclockid()       |      不支持      |                                                              |
|               |      pthread_getconcurrency()      |                  |                                                              |
|               |      pthread_getschedparam()       |      不支持      |                                                              |
|               |       pthread_getspecific()        | Smart-RISCV 支持 |                                                              |
|               |           pthread_join()           |      不支持      |                                                              |
|               |        pthread_key_create()        | Smart-RISCV 支持 |                                                              |
|               |        pthread_key_delete()        | Smart-RISCV 支持 |                                                              |
|               |      pthread_mutex_destroy()       | Smart-RISCV 支持 |                                                              |
|               |   pthread_mutex_getprioceiling()   |                  |                                                              |
|               |        pthread_mutex_init()        | Smart-RISCV 支持 |                                                              |
|               |        pthread_mutex_lock()        |      不支持      |                        5过2 1-1、2-1                         |
|               |   pthread_mutex_setprioceiling()   |      不支持      |                                                              |
|               |      pthread_mutex_trylock()       |      不支持      |                        7过2 1-1、3-1                         |
|               |       pthread_mutex_unlock()       | Smart-RISCV 支持 |                                                              |
|               |    pthread_mutexattr_destroy()     | Smart-RISCV 支持 |                                                              |
|               | pthread_mutexattr_getprioceiling() |      不支持      |                                                              |
|               |  pthread_mutexattr_getprotocol()   | Smart-RISCV 支持 |                                                              |
|               |    pthread_mutexattr_gettype()     | Smart-RISCV 支持 |                                                              |
|               |      pthread_mutexattr_init()      | Smart-RISCV 支持 |                                                              |
|               | pthread_mutexattr_setprioceiling() |                  |                                                              |
|               |  pthread_mutexattr_setprotocol()   | Smart-RISCV 支持 |                                                              |
|               |    pthread_mutexattr_settype()     | Smart-RISCV 支持 |                                                              |
|               |           pthread_once()           | Smart-RISCV 支持 |                                                              |
|               |           pthread_self()           | Smart-RISCV 支持 |                                                              |
|               |      pthread_setcancelstate()      | Smart-RISCV 支持 |                                                              |
|               |      pthread_setcanceltype()       |      不支持      |                                                              |
|               |      pthread_setconcurrency()      |                  |                                                              |
|               |      pthread_setschedparam()       |      不支持      |                                                              |
|               |       pthread_setschedprio()       |      不支持      |                                                              |
|               |       pthread_setspecific()        | Smart-RISCV 支持 |                                                              |
|               |        pthread_testcancel()        | Smart-RISCV 支持 |                                                              |
|    sched.h    |                                    |                  |                                                              |
|               |      sched_get_priority_max()      | Smart-RISCV 支持 | 1-3：Does not support SS (SPORADIC SERVER) 2-1：did no returned -1. |
|               |      sched_get_priority_min()      | Smart-RISCV 支持 | 1-3：Does not support SS (SPORADIC SERVER) 2-1：did no returned -1. |
|               |      sched_rr_get_interval()       |      不支持      | 1-1、2-1、3-1：sched_setscheduler failed: 38 (Function not implemented) |
|  semaphore.h  |                                    |                  |                                                              |
|               |            sem_close()             |      不支持      |                                                              |
|               |           sem_destroy()            |      不支持      |                                                              |
|               |           sem_getvalue()           |      不支持      |                                                              |
|               |             sem_init()             |      不支持      |                                                              |
|               |             sem_open()             |      不支持      |                                                              |
|               |             sem_post()             |      不支持      |                                                              |
|               |          sem_timedwait()           |      不支持      |                                                              |
|               |           sem_trywait()            |      不支持      |                                                              |
|               |            sem_unlink()            |      不支持      |                                                              |
|               |             sem_wait()             |      不支持      |                                                              |
|   setjmp.h    |                                    |                  |                                                              |
|               |             longjmp()              | Smart-RISCV 支持 |                                                              |
|               |              setjmp()              | Smart-RISCV 支持 |                                                              |
|   signal.h    |                                    |                  |                                                              |
|               |               kill()               |                  |                                                              |
|               |           pthread_kill()           |      不支持      |                        6过2 2-1、3-1                         |
|               |         pthread_sigmask()          |      不支持      |                       14过2 7-1、10-1                        |
|               |              raise()               | Smart-RISCV 支持 | 1-1、1-2、4-1：Should have exited from signal handler，Error waiting for child to exit 2-1：error：line 65 7-1：errno not correctly set 10000-1：errno not correctly set |
|               |            sigaction()             | Smart-RISCV 支持 |              测试用例太多（约400多），有的没过               |
|               |            sigaddset()             | Smart-RISCV 支持 |                                                              |
|               |            sigdelset()             | Smart-RISCV 支持 |                                                              |
|               |           sigemptyset()            | Smart-RISCV 支持 |                                                              |
|               |            sigfillset()            | Smart-RISCV 支持 |                                                              |
|               |           sigismember()            | Smart-RISCV 支持 |    5-1：Failed sigaddset(..., -2147483647) ret=0 errno=0     |
|               |              signal()              | Smart-RISCV 支持 | 3-1：Test FAILED: handler was called even though default was expected |
|               |            sigpending()            |      不支持      | 1-1：Not all pending signals found 1-2：This code should not be reachable 2-1：sigpending returned 0 when unsuccessful |
|               |           sigprocmask()            | Smart-RISCV 支持 | 4-1、5-1：FAIL: sigismember did not return 1 6-1：FAIL: Handler was not called for even though signal was removed from the signal mask 9-1：Handler wasn't called, implying signal was not delivered. |
|               |             sigqueue()             | Smart-RISCV 支持 | 1-1：Error waiting for child to exit 2-2：sigqueue() did not return -1 on ESRCH，At least one test FAILED -- see output for status 3-1、12-1：There is no other user than current and root.，Cannot run this test as non-root user 4-1、8-1：Test FAILED: 35 was queued 0 time(s) even though sigqueue was called 5 time(s) for 35 5-1：Test FAILED: 35 was not received even once 6-1：Test FAILED: signal was not delivered to process 10-1：sigqueue() did not return -1 on EINVAL 11-1：sigqueue() did not return -1 on ESRCH |
|               |            sigsuspend()            |      不支持      |                                                              |
|               |           sigtimedwait()           |      不支持      |                                                              |
|               |             sigwait()              |      不支持      |                                                              |
|               |           sigwaitinfo()            | Smart-RISCV 支持 | 1-1、9-1：SIGTOTEST is not pending 2-1：Test FAILED: sigwaitinfo() did not return the lowest of the multiple pending signals between SIGRTMIN and SIGRTMAX 3-1：Unexpected error while setting up test pre-conditions: No error information 5-1：Test FAILED: The selected signal number hasn't beenstored in the si_signo member. 7-1、8-1：sigwaitinfo() returned signal other than SIGTOTEST |
|   stdarg.h    |                                    |                  |                                                              |
|               |              va_arg()              | Smart-RISCV 支持 |                                                              |
|               |             va_copy()              | Smart-RISCV 支持 |                                                              |
|               |              va_end()              | Smart-RISCV 支持 |                                                              |
|               |             va_start()             | Smart-RISCV 支持 |                                                              |
|    stdio.h    |                                    |                  |                                                              |
|               |             clearerr()             | Smart-RISCV 支持 |                                                              |
|               |              fclose()              | Smart-RISCV 支持 |                                                              |
|               |              fdopen()              | Smart-RISCV 支持 |                                                              |
|               |               feof()               | Smart-RISCV 支持 |                                                              |
|               |              ferror()              | Smart-RISCV 支持 |                                                              |
|               |              fflush()              | Smart-RISCV 支持 |                                                              |
|               |              fgetc()               | Smart-RISCV 支持 |                                                              |
|               |              fgets()               | Smart-RISCV 支持 |                                                              |
|               |              fileno()              | Smart-RISCV 支持 |                                                              |
|               |            flockfile()             | Smart-RISCV 支持 |                                                              |
|               |              fopen()               | Smart-RISCV 支持 |                                                              |
|               |             fprintf()              | Smart-RISCV 支持 |                                                              |
|               |              fputc()               | Smart-RISCV 支持 |                                                              |
|               |              fputs()               | Smart-RISCV 支持 |                                                              |
|               |              fread()               | Smart-RISCV 支持 |                                                              |
|               |             freopen()              | Smart-RISCV 支持 |                                                              |
|               |              fscanf()              | Smart-RISCV 支持 |                                                              |
|               |           ftrylockfile()           |                  |                                                              |
|               |           funlockfile()            |                  |                                                              |
|               |              fwrite()              | Smart-RISCV 支持 |                                                              |
|               |               getc()               | Smart-RISCV 支持 |                                                              |
|               |          getc_unlocked()           |                  |                                                              |
|               |             getchar()              | Smart-RISCV 支持 |                                                              |
|               |         getchar_unlocked()         | Smart-RISCV 支持 |                                                              |
|               |               gets()               | Smart-RISCV 支持 |                                                              |
|               |              perror()              | Smart-RISCV 支持 |                                                              |
|               |              printf()              | Smart-RISCV 支持 |                                                              |
|               |               putc()               | Smart-RISCV 支持 |                                                              |
|               |          putc_unlocked()           |                  |                                                              |
|               |             putchar()              | Smart-RISCV 支持 |                                                              |
|               |         putchar_unlocked()         |                  |                                                              |
|               |               puts()               | Smart-RISCV 支持 |                                                              |
|               |              scanf()               | Smart-RISCV 支持 |                                                              |
|               |              setbuf()              |      不支持      |               setbuf() 失败，没办法暂存 buffer               |
|               |             setvbuf()              |      不支持      |               setvbuf()失败，没办法暂存 buffer               |
|               |             snprintf()             | Smart-RISCV 支持 |                                                              |
|               |             sprintf()              | Smart-RISCV 支持 |                                                              |
|               |              sscanf()              | Smart-RISCV 支持 |                                                              |
|               |               stderr               | Smart-RISCV 支持 |                                                              |
|               |               stdin                | Smart-RISCV 支持 |                                                              |
|               |               stdout               | Smart-RISCV 支持 |                                                              |
|               |              ungetc()              | Smart-RISCV 支持 |                                                              |
|               |             vfprintf()             | Smart-RISCV 支持 |                                                              |
|               |             vfscanf()              | Smart-RISCV 支持 |                                                              |
|               |             vprintf()              | Smart-RISCV 支持 |                                                              |
|               |              vscanf()              | Smart-RISCV 支持 |                                                              |
|               |            vsnprintf()             |      不支持      |                 调用后输出的buf会少一个字节                  |
|               |             vsprintf()             | Smart-RISCV 支持 |                                                              |
|               |             vsscanf()              | Smart-RISCV 支持 |                                                              |
|   stdlib.h    |                                    |                  |                                                              |
|               |              abort()               |      不支持      |                           程序卡死                           |
|               |               abs()                | Smart-RISCV 支持 |                                                              |
|               |               atof()               | Smart-RISCV 支持 |                                                              |
|               |               atoi()               | Smart-RISCV 支持 |                                                              |
|               |               atol()               | Smart-RISCV 支持 |                                                              |
|               |              atoll()               | Smart-RISCV 支持 |                                                              |
|               |             bsearch()              | Smart-RISCV 支持 |                                                              |
|               |              calloc()              | Smart-RISCV 支持 |                                                              |
|               |               div()                | Smart-RISCV 支持 |                                                              |
|               |               free()               | Smart-RISCV 支持 |                                                              |
|               |              getenv()              | Smart-RISCV 支持 |                                                              |
|               |               labs()               | Smart-RISCV 支持 |                                                              |
|               |               ldiv()               | Smart-RISCV 支持 |                                                              |
|               |              llabs()               | Smart-RISCV 支持 |                                                              |
|               |              lldiv()               | Smart-RISCV 支持 |                                                              |
|               |              malloc()              | Smart-RISCV 支持 |                                                              |
|               |              mktime()              | Smart-RISCV 支持 |                                                              |
|               |              qsort()               | Smart-RISCV 支持 |                                                              |
|               |               rand()               |      不支持      |          每次生成的数都是一样的，生成的是假的随机数          |
|               |              rand_r()              |      不支持      |          每次生成的数都是一样的，生成的是假的随机数          |
|               |             realloc()              | Smart-RISCV 支持 |                                                              |
|               |              setenv()              | Smart-RISCV 支持 |                                                              |
|               |              srand()               |      不支持      |          每次生成的数都是一样的，生成的是假的随机数          |
|               |              strtod()              | Smart-RISCV 支持 |                                                              |
|               |              strtof()              | Smart-RISCV 支持 |                                                              |
|               |              strtol()              | Smart-RISCV 支持 |                                                              |
|               |             strtold()              | Smart-RISCV 支持 |                                                              |
|               |             strtoll()              | Smart-RISCV 支持 |                                                              |
|               |             strtoul()              | Smart-RISCV 支持 |                                                              |
|               |             strtoull()             | Smart-RISCV 支持 |                                                              |
|               |             unsetenv()             | Smart-RISCV 支持 |                                                              |
|   string.h    |                                    |                  |                                                              |
|               |              memchr()              |                  |                                                              |
|               |              memcmp()              | Smart-RISCV 支持 |                                                              |
|               |              memcpy()              | Smart-RISCV 支持 |                                                              |
|               |             memmove()              | Smart-RISCV 支持 |                                                              |
|               |              memset()              | Smart-RISCV 支持 |                                                              |
|               |              strcat()              | Smart-RISCV 支持 |                                                              |
|               |              strchr()              | Smart-RISCV 支持 |                                                              |
|               |              strcmp()              | Smart-RISCV 支持 |                                                              |
|               |             strcoll()              | Smart-RISCV 支持 |                                                              |
|               |              strcpy()              | Smart-RISCV 支持 |                                                              |
|               |             strcspn()              | Smart-RISCV 支持 |                                                              |
|               |             strerror()             | Smart-RISCV 支持 |                                                              |
|               |            strerror_r()            | Smart-RISCV 支持 |                                                              |
|               |              strlen()              | Smart-RISCV 支持 |                                                              |
|               |             strncat()              | Smart-RISCV 支持 |                                                              |
|               |             strncmp()              | Smart-RISCV 支持 |                                                              |
|               |             strncpy()              | Smart-RISCV 支持 |                                                              |
|               |             strpbrk()              | Smart-RISCV 支持 |                                                              |
|               |             strrchr()              | Smart-RISCV 支持 |                                                              |
|               |              strspn()              | Smart-RISCV 支持 |                                                              |
|               |              strstr()              | Smart-RISCV 支持 |                                                              |
|               |              strtok()              | Smart-RISCV 支持 |                                                              |
|               |             strtok_r()             | Smart-RISCV 支持 |                                                              |
|               |             strxfrm()              | Smart-RISCV 支持 |                                                              |
|  sys/mman.h   |                                    |                  |                                                              |
|               |             mlockall()             | Smart-RISCV 支持 | 3-6：An error occurs when calling shm_open(): No such device 3-7：An error occurs when calling mmap(): Operation not permitted 13-1、13-2：mlockall() return 0 instead of -1. |
|               |               mmap()               | Smart-RISCV 支持 | 5-1：Combination of PROT_NONE with MAP_SHARED has failed: Operation not permitted 6-4：Maping readonly file with PROT_WRITE, MAP_SHARED have not returned EACCES 6-6：Mapping writeonly file with PROT_READ have not returned EACCES 1-1、3-1、7-1、6-1、6-2、6-3、6-5、7-2、10-1、11-2：Error at mmap(): Operation not permitted 1-2、7-3、7-4、24-1、24-2：Error at shm_open(): No such device 9-1：pa is not multiple of page_size 11-3、11-5：Error at shm_open(): No such device 11-6：Error at read(): Bad file descriptor 11-4、12-1、14-1、27-1：Error at mmap: Operation not permitted 13-1：Couldn't mount /proc/mounts 18-1：Error at setrlimit(): Function not implemented 19-1：Test FAILED: Did not get EBADF when fd is invalid 23-1、32-1：Test FAILED: Expect ENODEV, get: Operation not permitted 31-1：UNSUPPORTED: Cannot be tested on 64 bit architecture |
|               |             munlock()              | Smart-RISCV 支持 | 10-1：Unexpected error: No error information 11-1：munlock() does not require that addr be a multiple of {PAGESIZE}. |
|               |              munmap()              |      不支持      |                                                              |
|               |             shm_open()             | Smart-RISCV 支持 | 8-1、11-1、13-1、22-1、25-1、26-1、26-2、28-3：An error occurs when calling shm_open(): No such device 20-1、20-2、21-1、41-1：An error occurs when calling shm_unlink(): Function not implemented 23-1：error at sem_open: No such device 1-1、5-1、14-2、28-1、28-2：An error occurs when calling shm_open(): No such device 37-1：char which are in portable character set, but not in portable filename character set 38-1：shm_open: No such device 39-2：UNTESTED: shm_open() did not fail with ENAMETOLONG 16-1、17-1、18-1、20-3：An error occurs when calling shm_unlink(): Function not implemented |
|               |            shm_unlink()            | Smart-RISCV 支持 | 1-1、8-1、9-1：An error occurs when calling shm_open(): No such device 2-1、3-1、5-1、6-1：An error occurs when calling shm_open(): No such device 10-2：UNTESTED: shm_open() did not fail with ENAMETOLONG 11-1：Unexpected error: Function not implemented |
| sys/utsname.h |                                    |                  |                                                              |
|               |              uname()               |                  |                                                              |
|    time.h     |                                    |                  |                                                              |
|               |             asctime()              | Smart-RISCV 支持 |                                                              |
|               |            asctime_r()             |                  |                                                              |
|               |           clock_getres()           | Smart-RISCV 支持 | 5-1、6-1：clock_getres() did not return -1 6-2：clock_getres(-2147483648, &res); |
|               |          clock_gettime()           | Smart-RISCV 支持 | 8-2：errno not correctly set 1-1、1-2、2-1、3-1、4-1：clock_gettime() failed 8-1：errno not set to EINVAL |
|               |         clock_nanosleep()          |      不支持      |                                                              |
|               |          clock_settime()           | Smart-RISCV 支持 | 4-1、4-2、5-1、5-2、7-1、7-2、8-1、19-1：clock_gettime() did not return success 17-1、17-2、20-1：errno != EINVAL |
|               |              ctime()               | Smart-RISCV 支持 |                                                              |
|               |             ctime_r()              |                  |                                                              |
|               |             difftime()             | Smart-RISCV 支持 |                                                              |
|               |              gmtime()              | Smart-RISCV 支持 |                                                              |
|               |             gmtime_r()             |                  |                                                              |
|               |            localtime()             | Smart-RISCV 支持 |                                                              |
|               |           localtime_r()            |                  |                                                              |
|               |            nanosleep()             |      不支持      |                                                              |
|               |             strftime()             |      不支持      |                                                              |
|               |               time()               | Smart-RISCV 支持 |                                                              |
|               |           timer_create()           | Smart-RISCV 支持 | 1-1、7-1：nanosleep() not interrupted 8-1：Error waiting for child to exit 11-1：clock_gettime() failed 16-1：timer_create returned success |
|               |           timer_delete()           | Smart-RISCV 支持 |    1-2：timer_settime() did not fail after timer_delete()    |
|               |         timer_getoverrun()         |      不支持      |                                                              |
|               |          timer_gettime()           | Smart-RISCV 支持 | 1-1、1-2：timer_gettime() value !~= time expected left 1-3：timer_gettime() deltans: 400000000 allowed: 30000000 1-4：it_interval not correctly set 3-1：clock_gettime() did not return success |
|               |          timer_settime()           | Smart-RISCV 支持 | 1-1、1-2：nanosleep() not interrupted 2-1、5-1、5-3、6-1、9-1、9-2：clock_gettime() did not return success 5-2：signal was not sent 8-2：value: tv_sec 1000 tv_nsec 1000 8-3：time left 5 oits.it_value.tv_sec 0 8-4：Unhandled Exception 13:Load Page Fault scause:0x000000000000000d stval:0x00000000000000a6 sepc:0x000000020000da88 13-1：timer_settime() did not return failure |
|               |               tzname               |                  |                      未有专门的测试用例                      |
|               |              tzset()               |                  |                      未有专门的测试用例                      |
|   unistd.h    |                                    |                  |                                                              |
|               |              alarm()               | Smart-RISCV 支持 |                                                              |
|               |              close()               | Smart-RISCV 支持 |                                                              |
|               |              environ               |                  |                                                              |
|               |            fdatasync()             | Smart-RISCV 支持 |                                                              |
|               |              fsync()               | Smart-RISCV 支持 | 5-1：Expect EBADF, get: Operation not permitted 7-1：Test Fail: Expect EINVAL, get: Function not implemented |
|               |              pause()               |                  |                      未有专门的测试用例                      |
|               |               read()               | Smart-RISCV 支持 |                                                              |
|               |             sysconf()              |                  |                      未有专门的测试用例                      |
|               |              write()               | Smart-RISCV 支持 |                                                              |
|               |             confstr()              |                  |                      未有专门的测试用例                      |

# 未测试：

| C库       | 函数                               |        备注        |
| --------- | ---------------------------------- | :----------------: |
| pthread.h |                                    | 未有专门的测试用例 |
|           | pthread_attr_getguardsize()        | 未有专门的测试用例 |
|           | pthread_attr_getstackaddr()        | 未有专门的测试用例 |
|           | pthread_attr_setguardsize()        | 未有专门的测试用例 |
|           | pthread_getconcurrency()           | 未有专门的测试用例 |
|           | pthread_mutex_getprioceiling()     | 未有专门的测试用例 |
|           | pthread_mutexattr_setprioceiling() | 未有专门的测试用例 |
|           | pthread_setconcurrency()           | 未有专门的测试用例 |
| stdio.h   |                                    | 未有专门的测试用例 |
|           | ftrylockfile()                     | 未有专门的测试用例 |
|           | funlockfile()                      | 未有专门的测试用例 |
|           | getc_unlocked()                    | 未有专门的测试用例 |
|           | putc_unlocked()                    | 未有专门的测试用例 |
|           | putchar_unlocked()                 | 未有专门的测试用例 |
| time.h    |                                    | 未有专门的测试用例 |
|           | asctime_r()                        | 未有专门的测试用例 |
|           | gmtime_r()                         | 未有专门的测试用例 |
|           | localtime_r()                      | 未有专门的测试用例 |
|           | tzname()                           | 未有专门的测试用例 |
|           | tzset()                            | 未有专门的测试用例 |
| unistd.h  |                                    | 未有专门的测试用例 |
|           | environ()                          | 未有专门的测试用例 |
|           | pause()                            | 未有专门的测试用例 |
|           | sysconf()                          | 未有专门的测试用例 |
|           | confstr()                          | 未有专门的测试用例 |


#ifndef _RTT_SYS_SYSCALL_H
#define _RTT_SYS_SYSCALL_H

long syscall(int number, ...);

#define NRSYS(x) _NRSYS_##x,
enum
{
    _NRSYS_NONE = 0,

    #include "syscall_no.h"

    _NRSYS_SYSCALL_NR
};

#endif

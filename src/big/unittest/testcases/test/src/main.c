/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-06     QuanZhao     the first version
 */
#include <stdio.h>
#include <string.h>     /* strcmp */

#include "utest.h"
int __clone(int (*func)(void *), void *stack, int flags, void *arg, ...){return 0;}

static void usage(char *name)
{
    printf("Usage: %s (list|self|all|<test_cases>)\n"
            "   list: list all the testcases\n"
            "   self: test the utest itself\n"
            "   all : run all the testcases\n"
            "   <test_case list>: run the specified testcases\n", name);
}

int main(int argc, char** argv)
{
    int i = 0;

    utest_init();

    if (argc == 1)
    {
        usage(argv[0]);
        return 0;
    }

    if (argc == 2)
    {
        if (strcmp(argv[1], "list") == 0)   /* list all the testcases */
        {
            utest_tc_list();
            return 0;
        }

        if (strcmp(argv[1], "self") == 0)   /* test myself */
        {
            utest_run("self.*");
            return 0;
        }

        if (strcmp(argv[1], "all") == 0)    /* run all the testcases */
        {
            utest_run("utest.*");
            return 0;
        }
    }

    for (i = 1; i < argc; i++)
    {
        LOG_I("run testcase: %s\n", argv[i]);
        utest_run(argv[i]);
    }
    return 0;
}

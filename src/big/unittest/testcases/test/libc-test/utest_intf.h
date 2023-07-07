#ifndef UTEST_INTF_H
#define UTEST_INTF_H

#include <utest.h>

#ifndef GROUPNAME
#define GROUPNAME "null"
#endif

#ifndef TCNAME
#define TCNAME "null"
#endif

#ifndef main
#define main(x...) musl_tc(void)
#endif

static int musl_tc(void);

static void TESTCASE(void)
{
    int result = musl_tc();
    uassert_int_equal(result, 0);
}

static rt_err_t utest_tc_init(void)
{
#ifdef USING_T_STATUS
    t_status = 0;
#endif
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(TESTCASE);
}

#ifndef SUBGROUP
UTEST_TC_EXPORT(testcase, "utest.libc_test." GROUPNAME "." TCNAME, utest_tc_init, utest_tc_cleanup, 5);
#else
UTEST_TC_EXPORT(testcase, "utest.libc_test." GROUPNAME "." SUBGROUP "." TCNAME, utest_tc_init, utest_tc_cleanup, 5);
#endif

#endif

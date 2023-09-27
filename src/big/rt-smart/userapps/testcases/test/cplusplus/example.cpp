#include <stdio.h>
#include <utest.h>

#include "example.h"

#define GLOBAL_VALUE    (10)
#define LOCAL_VALUE     (100)

Example global_obj(GLOBAL_VALUE);

Example::Example()
    : value(0)
{
}

Example::Example(int a)
    : value(a)
{
}

void Example::setValue(int a)
{
}

int Example::getValue(void)
{
    return this->value;
}

extern "C"
{

    static void cpp_test(void)
    {
        Example local_obj(LOCAL_VALUE);

        uassert_int_equal(local_obj.getValue(), LOCAL_VALUE);
        uassert_int_equal(global_obj.getValue(), GLOBAL_VALUE);
    }

    static rt_err_t utest_tc_init(void)
    {
        return RT_EOK;
    }

    static rt_err_t utest_tc_cleanup(void)
    {
        return RT_EOK;
    }

    static void testcase(void)
    {
        UTEST_UNIT_RUN(cpp_test);
    }

    UTEST_TC_EXPORT(testcase, "utest.cpp_test.cpp_tc", utest_tc_init, utest_tc_cleanup, 5);
}


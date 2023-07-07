/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Test that clock_gettime() returns -1 on failure.
 */
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"
#define TESTTIME 1037128358
#define INVALIDCLOCK 9999
int main(void)
{
	struct timespec tp;
	struct timespec tpset;
	struct timespec ret_tpset;
	tpset.tv_sec = TESTTIME;
	tpset.tv_nsec = 0;

	if (clock_gettime(INVALIDCLOCK, &tp) != -1)
	{
		printf("{Test Fail}:clock_gettime\n");
		return PTS_FAIL;
	}

#ifdef CLOCK_MONOTONIC
	if (clock_gettime(CLOCK_MONOTONIC, &ret_tpset) == -1)
	{
		printf("{Test Fail}:clock_gettime\n");
		return PTS_FAIL;
	}
	printf("CLOCK_MONOTONIC ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);
	sleep(1);
	if (clock_gettime(CLOCK_MONOTONIC, &ret_tpset) == -1)
	{
		printf("{Test Fail}:clock_gettime\n");
		return PTS_FAIL;
	}
	printf("CLOCK_MONOTONIC ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);
#else
	printf("CLOCK_MONOTONIC not supported\n");
	printf("Test FAILED\n");
	return PTS_FAIL;
#endif

#ifdef CLOCK_REALTIME
	if (clock_gettime(CLOCK_REALTIME, &ret_tpset) == -1)
	{
		printf("{Test Fail}:clock_gettime\n");
		return PTS_FAIL;
	}
	printf("CLOCK_REALTIME ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);
	if (clock_settime(CLOCK_REALTIME, &tpset) == -1)
	{
		printf("clock_settime() fail with CLOCK_REALTIME\n");
		return PTS_FAIL;
	}
	if (clock_gettime(CLOCK_REALTIME, &ret_tpset) == -1)
	{
		printf("{Test Fail}:clock_gettime\n");
		return PTS_FAIL;
	}
	printf("CLOCK_REALTIME ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);

	int time1 = 0, time2 = 0;
	if (clock_gettime(CLOCK_REALTIME, &ret_tpset) == -1)
	{
		printf("{Test Fail}:clock_gettime\n");
		return PTS_FAIL;
	}
	time2 = ret_tpset.tv_sec;
	for (int i = 0; i < 10; i++)
	{
		sleep(1);
		if (clock_gettime(CLOCK_REALTIME, &ret_tpset) == -1)
		{
			printf("{Test Fail}:clock_gettime\n");
			return PTS_FAIL;
		}
		time1 = ret_tpset.tv_sec;
		printf("CLOCK_REALTIME ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);
		if (time1 - time2 != 1)
		{
			printf("after sleep time not change or not right");
			return PTS_FAIL;
		}
		time2 = time1;
	}
#else
	printf("CLOCK_REALTIME not supported\n");
	printf("Test FAILED\n");
	return PTS_FAIL;
#endif
	printf("{Test PASSED}\n");
	return PTS_PASS;
}

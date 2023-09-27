/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_settime() cannot set the monotonic clock CLOCK_MONOTONIC,
 * and will fail if invoked with clock CLOCK_MONOTONIC.
 *
 * The date chosen is Nov 12, 2002 ~11:13am.
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

#define TESTTIME 1037128358

int main(void)
{
	struct timespec tpset;
	struct timespec ret_tpset;
	tpset.tv_sec = TESTTIME;
	tpset.tv_nsec = 0;
#ifdef CLOCK_MONOTONIC
	clock_gettime(CLOCK_MONOTONIC, &ret_tpset);
	printf("CLOCK_MONOTONIC ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);
	if (clock_settime(CLOCK_MONOTONIC, &tpset) == -1)
	{
		printf("clock_settime() with CLOCK_MONOTONIC return -1\n");
	}
	else
	{
		printf("clock_settime() did not fail with CLOCK_MONOTONIC\n");
		return PTS_FAIL;
	}
	clock_gettime(CLOCK_MONOTONIC, &ret_tpset);
	printf("CLOCK_MONOTONIC ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);
#else
	printf("CLOCK_MONOTONIC not supported\n");
#endif

#ifdef CLOCK_REALTIME
	clock_gettime(CLOCK_REALTIME, &ret_tpset);
	printf("CLOCK_REALTIME ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);
	if (clock_settime(CLOCK_REALTIME, &tpset) == -1)
	{
		printf("clock_settime() fail with CLOCK_REALTIME\n");
		return PTS_FAIL;
	}
	clock_gettime(CLOCK_REALTIME, &ret_tpset);
	printf("CLOCK_REALTIME ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);

	int time1 = 0, time2 = 0;
	clock_gettime(CLOCK_REALTIME, &ret_tpset);
	time2 = ret_tpset.tv_sec;
	for (int i = 0; i < 10; i++)
	{
		sleep(1);
		clock_gettime(CLOCK_REALTIME, &ret_tpset);
		time1 = ret_tpset.tv_sec;
		if(time1 - time2 != 1)
		{
			printf("after sleep time not change or not right");
			return PTS_FAIL;
		}
		time2 = time1;
	}
#else
	printf("CLOCK_REALTIME not supported\n");
#endif
	printf("{Test PASSED}\n");
	return PTS_PASS;
}

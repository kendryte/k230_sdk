/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * This test case checks if the return value of the ctime call is
 * not NULL after converting the time value to a date and time string.
 */

#include <stdio.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{
	time_t current_time;
	char *result;
	char str[32];
	time(&current_time);
	result = ctime_r(&current_time, str);
	if (result == NULL)
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	printf("converted date and time is: %s\n", str);
	printf("{Test PASSED}\n");
	return PTS_PASS;
	
}

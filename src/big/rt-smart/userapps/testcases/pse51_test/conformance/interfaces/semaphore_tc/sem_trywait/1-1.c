/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
 * unamed semaphore is used in subsequent of sem_wait.
*/

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "sem_trywait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t mysemp;
	int val;

	if (sem_init(&mysemp, 0, 1) == -1) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	if (sem_trywait(&mysemp) == -1) {
		perror(ERROR_PREFIX "trywait");
		return PTS_UNRESOLVED;
	}

	if (val <= 0) {
		puts("{Test PASSED}");
		sem_destroy(&mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "posixtest.h"

int main(void)
{
	struct timespec ret_tpset;
    int ret =0;
	int time1 = 0, time2 = 0;
	clock_gettime(CLOCK_REALTIME, &ret_tpset);
	time2 = ret_tpset.tv_sec;
	for (int i = 0; i < 10; i++)
	{
		ret = sleep(1);
        printf("sleep ret = %d\n",ret);
		if(ret != 0)
		{
			printf("error : sleep not finished");
			return PTS_FAIL;
		}
		clock_gettime(CLOCK_REALTIME, &ret_tpset);
		time1 = ret_tpset.tv_sec;
		printf("CLOCK_REALTIME ret_tpset.tv_sec:%ld\n", ret_tpset.tv_sec);
		if(time1 - time2 != 1)
		{
			printf("error : after sleep time not change or not right");
			return PTS_FAIL;
		}
		time2 = time1;
	}
	printf("{Test PASSED}\n");
	return PTS_PASS;
}

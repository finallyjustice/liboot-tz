// gcc nanosecond.c -o nanosecond -lrt

#include <stdio.h>
#include <time.h>

int main(int argc, char **argv)
{        
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
    
	printf("CLOCK_REALTIME: %d, %d\n", ts.tv_sec, ts.tv_nsec);
    
	return 0;
}

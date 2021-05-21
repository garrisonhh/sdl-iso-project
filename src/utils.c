#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include "utils.h"

struct timeval LAST_TIMEIT = {
	.tv_sec = 0, 
	.tv_usec = 0
};

bool d_close(double a, double b) {
	return fabs(a - b) < FLOAT_TOLERANCE;
}

void timeit_start() {
	gettimeofday(&LAST_TIMEIT, NULL);
}

void timeit_end(const char *message) {
	struct timeval time_now, time_diff;

	gettimeofday(&time_now, NULL);

	time_diff.tv_sec = time_now.tv_sec - LAST_TIMEIT.tv_sec;
	time_diff.tv_usec = time_now.tv_usec - LAST_TIMEIT.tv_usec;

	if (time_diff.tv_usec < 0) {
		--time_diff.tv_sec;
		time_diff.tv_usec += 1000000;
	}

	char us[7];

	us[6] = 0;

	sprintf(us, "%6li", time_diff.tv_usec);
	
	for (int i = 0; i < 6; i++)
		if (us[i] == ' ')
			us[i] = '0';

	printf("timeit");

	if (message != NULL)
		printf(" %s", message);

	printf(":\t%ld.%ss\n", time_diff.tv_sec, us);

	LAST_TIMEIT = time_now;
}

double timeit_get_time() {
	struct timeval now;

	gettimeofday(&now, NULL);

	return (double)now.tv_sec + ((double)now.tv_usec) / 1000000;
}

void print_bits(const char *message, unsigned n, size_t bits) {
	if (message != NULL)
		printf("%s:\t", message);

	for (int i = bits - 1; i >= 0; --i)
		printf("%u", BIT_GET(n, i));

	printf("\n");
}

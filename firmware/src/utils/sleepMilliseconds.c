#include <time.h>
#include "../../include/utils/sleep_milliseconds.h"
void sleep_ms(unsigned int delayMs)
{
	const unsigned int NS_PER_MS = 1000 * 1000;
	const unsigned int NS_PER_SECOND = 1000000000;

	unsigned long long delayNs = delayMs * NS_PER_MS;
	int seconds = delayNs / NS_PER_SECOND;
	int nanoseconds = delayNs % NS_PER_SECOND;

	struct timespec reqDelay = {seconds, nanoseconds};
	nanosleep(&reqDelay, (struct timespec *) NULL);
}

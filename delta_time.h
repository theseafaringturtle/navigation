#include <unistd.h>
#include <time.h>
#include <sys/time.h>


int timeval_subtract(struct timespec *result, struct timespec *x, struct timespec *y);

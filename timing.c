#include <time.h>

void msleep(unsigned long milisec) {
	struct timespec req={0};
	time_t sec=(int)(milisec/1000);
	milisec=milisec-(sec*1000);
	req.tv_sec=sec;
	req.tv_nsec=milisec*1000000l;
	nanosleep(&req, NULL);
}
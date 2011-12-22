#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/kd.h>
#include "pcspkr.h"

void spkr_init() {
	//there must be a better way than this.. that won't require root
	console=open("/dev/console",O_WRONLY);
}
void spkr_on(float freq) {
	ioctl(console, KIOCSOUND, (int)(1193180.0/freq));
	//ioctl(STDOUT_FILENO, KIOCSOUND, (int)(1193180.0/freq));
}
void spkr_off() {
	ioctl(console, KIOCSOUND, 0);
	//ioctl(STDOUT_FILENO, KIOCSOUND, 0);
}

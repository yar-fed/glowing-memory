#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

inline int read_event(int fd, struct input_event *e, size_t ev_size)
{
	ssize_t res = read(fd, e, ev_size);
	if (res < 0) {
		printf("Error reading event");
		return res;
	}
	return 0;
}

int grab_keypad(char *kp)
{
	int fd = open(kp, O_RDONLY);
	if (fd < 0)
		printf("Error grabing keypad");
	return fd;
}

void release_keypad(int fd)
{
	close(fd);
}

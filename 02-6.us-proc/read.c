#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 10

int main(void)
{
	char buf[BUF_SIZE];
	int fd = open("/proc/proc-status", O_RDONLY);
	while ( read(fd, buf, BUF_SIZE) )
		puts(buf);
	return 0;
}	

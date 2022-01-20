#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 10

int main(void)
{
	char *hello_str="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int fd = open("/proc/proc-status", O_RDWR);
	char *pos = hello_str;
	for (pos=hello_str; pos < hello_str+strlen(hello_str); pos += BUF_SIZE) 
		write(fd, pos, BUF_SIZE);
	write(fd,"\n",1);
	close(fd);
	return 0;
}	

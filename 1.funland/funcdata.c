#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long




//
static u8 funcdatabuf[0x100000];
static int funcdatafd;
static int funcdatalen;




void funcdata_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/func.data";

	if(flag == 0)
	{
		funcdatafd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		funcdatalen = 0;

		buf = (void*)funcdatabuf;
		for(j=0;j<0x100000;j++)buf[j] = 0;
	}
	else
	{
		//open
		funcdatafd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		funcdatalen = read(funcdatafd, funcdatabuf, 0x100000);
		printf("funcdata:	%x\n", funcdatalen);

		//clean
		buf = (void*)funcdatabuf;
		for(j=funcdatalen;j<0x100000;j++)buf[j] = 0;
	}
}
void funcdata_stop()
{
}
void funcdata_create()
{
}
void funcdata_delete()
{
	lseek(funcdatafd, 0, SEEK_SET);
	write(funcdatafd, funcdatabuf, funcdatalen);
	close(funcdatafd);
}

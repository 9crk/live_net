#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
/*
extern int writerSetBuffer(int shareID, char** buffer);
extern int readerGetBuffer(int shareID, char** buffer);
extern unsigned int writeBuffer(char* circleBuff, char* data,unsigned int length);
extern int readBuffer(char* circleBuff, char* data,int datalen);
*/
int main(int argc, char*argv[])
{
	int cnt,ret;
	char* buff1;
	char data[1000000];
	FILE*fp = fopen(argv[1], "rb");
	system("touch zhou.264");
	//unlink( "zhou.264" );
	//mkfifo( "zhou.264", 0777 );
	writerSetBuffer(1342, &buff1);
	while(1){
		cnt = fread(data, 1, 100000, fp);
		if(cnt == 0){rewind(fp);continue;}
		ret = writeBuffer(buff1, data, cnt);
		printf("write %d\n", cnt);
		
		while(ret == 0){
			usleep(200000);
			printf(".");
			ret = writeBuffer(buff1, data, cnt);
		}		
	}
	fclose(fp);
}

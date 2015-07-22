#include <sys/shm.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#define BUFFER_LEN  (1024*1024 - 12)
#define SHARED_BUFFER_LEN (BUFFER_LEN + 12)
#define TRUE 1
#define FALSE 0
#pragma pack(push) 
#pragma pack(4)
 typedef struct {
	unsigned int readIndex;
	unsigned int writeIndex;
	unsigned int allowWrite;
	char buffer[BUFFER_LEN];
}circleBuffer;
#pragma pack(pop) 

void* membuff;

int writerSetBuffer(int shareID, char** buffer)
{
	int shmid;
	circleBuffer **buff = (circleBuffer**)buffer; 
	shmid =shmget( shareID,SHARED_BUFFER_LEN,0666|IPC_CREAT );    
	if(shmid >= 0){
		printf( "Create a shared memory segment %d\n", shmid);  
	}
	membuff = shmat( shmid, ( const void* )0,0 );  
	if(membuff == (void*)-1){
		printf("shmat error: %s!\n", strerror(errno));
		return -1;
	}
	*buff = (circleBuffer*)membuff;
	(*buff)->allowWrite = 1;
	(*buff)->readIndex = 0;
	(*buff)->writeIndex = 0;
	return shmid;
}
int readerGetBuffer(int shareID, char** buffer)
{
	int shmid;
	circleBuffer **buff = (circleBuffer**)buffer; 
	shmid =shmget( shareID,0,0666);    
	if( shmid >= 0 ) { 
		printf( "Create a shared memory segment %d\n", shmid);  
	}
	membuff = shmat( shmid, (const void*)0,0 );  
	if(membuff == (void*)-1){
		printf("shmat error: %s!\n", strerror(errno));
	}
	*buff = (circleBuffer*)membuff;
	return shmid;
}

int readBuffer(char* circleBuff, char* data,int datalen)
{
	circleBuffer* circleBuf =(circleBuffer*)circleBuff; 
	unsigned int writeIndex = circleBuf->writeIndex;
		unsigned int readIndex  = circleBuf->readIndex;
	int ret;
	//printf("WriteIndex is %d readIndex is %d Del=%d\n",writeIndex, readIndex,datalen);
	if (readIndex < writeIndex){								
		if (readIndex + datalen < writeIndex){				
			memcpy(data, circleBuf->buffer + readIndex, datalen);
				circleBuf->readIndex += datalen;
			ret = datalen;
		}else{									
			memcpy(data, circleBuf->buffer + readIndex, writeIndex - readIndex);
			circleBuf->readIndex = writeIndex;
circleBuf->allowWrite = TRUE;
printf("no data...\n");
ret = writeIndex - readIndex;
		}
	}else if (readIndex > writeIndex){
		if (readIndex + datalen < BUFFER_LEN){		
			memcpy(data, circleBuf->buffer + readIndex, datalen);
			circleBuf->readIndex += datalen;
			ret = datalen;
		}else{													
			if (readIndex + datalen - BUFFER_LEN < writeIndex){
				memcpy(data, circleBuf->buffer + readIndex, BUFFER_LEN - readIndex);
				memcpy(data + BUFFER_LEN - readIndex, circleBuf->buffer, readIndex + datalen - BUFFER_LEN);		
				circleBuf->readIndex = readIndex + datalen - BUFFER_LEN;
				ret = datalen;
			}else{											 
				printf("no data...\n");			
					circleBuf->allowWrite = TRUE;
					memcpy(data, circleBuf->buffer + readIndex, BUFFER_LEN - readIndex);
				memcpy(data + BUFFER_LEN - readIndex, circleBuf->buffer, writeIndex);
				circleBuf->readIndex = writeIndex;
				ret = writeIndex + BUFFER_LEN - readIndex;
			}
		}
	}else{													
		circleBuf->allowWrite = TRUE;
		ret = 0;
	}
	return ret;
}
void clearBuffer(char* circleBuff)
{
	circleBuffer* circleBuf =(circleBuffer*)circleBuff;
    circleBuf->readIndex = circleBuf->writeIndex;
}
/*
int readBufferEx(circleBuffer* circleBuf, char* data)
{
	unsigned int writeIndex = circleBuf->writeIndex;
	unsigned int readIndex  = circleBuf->readIndex;
	int ret;
	//printf("WriteIndex is %d readIndex is %d Del=%d\n",writeIndex, readIndex,datalen);
	if (readIndex < writeIndex){								
		if (readIndex + datalen < writeIndex){					//------------r---l---w-------------
			memcpy(data, circleBuf->buffer + readIndex, datalen);
			circleBuf->readIndex += datalen;
			ret = datalen;
		}else{													//------------r------w----l--------------
			memcpy(data, circleBuf->buffer + readIndex, writeIndex - readIndex);
			circleBuf->readIndex = writeIndex;
			circleBuf->allowWrite = TRUE;
			printf("no data...\n");
			ret = writeIndex - readIndex;
		}
	}else if (readIndex > writeIndex){
		if (readIndex + datalen < BUFFER_LEN){					//----w----------------------r-----l----
			memcpy(data, circleBuf->buffer + readIndex, datalen);
			circleBuf->readIndex += datalen;
			ret = datalen;
		}else{													
			if (readIndex + datalen - BUFFER_LEN < writeIndex){//---l----w------------------------r----
				memcpy(data, circleBuf->buffer + readIndex, BUFFER_LEN - readIndex);
				memcpy(data + BUFFER_LEN - readIndex, circleBuf->buffer, readIndex + datalen - BUFFER_LEN);		
				circleBuf->readIndex = readIndex + datalen - BUFFER_LEN;
				ret = datalen;
			}else{											  //--w----l--------------------------r---
				printf("no data...\n");			
				circleBuf->allowWrite = TRUE;
				memcpy(data, circleBuf->buffer + readIndex, BUFFER_LEN - readIndex);
				memcpy(data + BUFFER_LEN - readIndex, circleBuf->buffer, writeIndex);
				circleBuf->readIndex = writeIndex;
				ret = writeIndex + BUFFER_LEN - readIndex;
			}
		}
	}else{													//-----------w==r--------------------------
		circleBuf->allowWrite = TRUE;
		ret = 0;
	}
	return ret;
}*/
/*
返回：
0   不允许写，r+l > w
0   不允许写，r = w
n   写入了n个字节的数据
*/
//返回是否成功              被存储的buffer       数据          长度
unsigned int writeBuffer(char* circleBuff, char* data,unsigned int length){
	circleBuffer* circleBuf =(circleBuffer*)circleBuff; 
	unsigned int writeIndex = circleBuf->writeIndex;
	unsigned int readIndex = circleBuf->readIndex;
	
//	printf("w is %d  r is %d Add %d bytes\n",writeIndex,readIndex,length);
	if (writeIndex >= readIndex){
		if (writeIndex == readIndex && circleBuf->allowWrite == FALSE){
			printf("full, r==w\n");
			return 0;
		}
		if ((writeIndex + length) > BUFFER_LEN){				
			if (writeIndex + length - BUFFER_LEN < readIndex){		//----l--r-------------------w--
				memcpy(circleBuf->buffer + writeIndex, data, BUFFER_LEN - writeIndex);
				memcpy(circleBuf->buffer, data + BUFFER_LEN - writeIndex, writeIndex + length - BUFFER_LEN);
				circleBuf->writeIndex = writeIndex + length - BUFFER_LEN;
			}else{													//---r--l---------------------w-
				printf("full, w+l > r\n");
				return 0;
			}
		}else{														//-----r-----------------w----l--
			memcpy(circleBuf->buffer + writeIndex, data, length);
			circleBuf->writeIndex += length;
		}
	}else if (writeIndex < readIndex){								
		if (writeIndex + length >= readIndex){						//------------w--r--l-----------应当丢失，报错，返回0
			printf("full, w+l > r\n");
			return 0;
		}else{														//------------w---l---r----------正常，返回l
			memcpy(circleBuf->buffer + writeIndex, data, length);
			circleBuf->writeIndex += length;
		}
	}
	return length;
}
/*
//reader  test
int main()
{
char* p;
int a=readerGetBuffer(1342,&p);
char data[100000];
int i;
int n;
	while(1)
	{
		n = readBuffer(p,data, 100000);
		if(n>0){
			for(i=0;i<10;i++)printf("%c", data[i]);
			printf("\n");
		}
		printf("I read %d bytes\n", n);
		sleep(1);
	}
}

//writer test
int main()
{
char* p;
int a=writerSetBuffer(1342,&p);
char data[100000];
int i;
	while(1)
	{
		memset(data, 0, 1000);
		scanf("%s", data);
		writeBuffer(p,data, 100000);
		for(i=0;i<10;i++)printf("%c", data[i]);
		printf("\n");
	}
}
*/

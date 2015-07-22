#include <stdio.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <unistd.h>  
#include <fcntl.h>  
  
  
#define PATHNAME "/tmp/H264_fifo"  
  
int main()  
{  
    int writefifo;  
    writefifo = open(PATHNAME,O_WRONLY,0);    //打开刚刚创建的FIFO文件  
    char str[1024];  
    char buf[1024];  
    while (scanf("%s",str)!=EOF)          //从终端输入信息  
    {  
        snprintf(buf,1024,"%s,write by process %d",str,getpid());  
        write(writefifo,buf,strlen(buf)+1);   //写入到fifo文件中去  
    }  
    close(writefifo);  
    return 0;  
}  

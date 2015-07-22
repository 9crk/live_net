#include <stdio.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <unistd.h>  
#include <fcntl.h>  
  
#define PATHNAME "/tmp/H264_fifo"  
  
  
int main()  
{  
    int readfifo;  
    readfifo = open(PATHNAME,O_RDONLY,0);    //打开FIFO文件进行读  
    char str[1024];  
    while (read(readfifo,str,1024) > 0)   //从FIFO文件中读取信息  
    {  
        printf("Process %d read:\n%s\n",getpid(),str);    //打印出来  
    }  
    close(readfifo);  
    return 0;  
}  

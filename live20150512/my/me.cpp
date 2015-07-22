#include <stdio.h>
#include<stdlib.h>
#include "RTSPStream.h"
#include<unistd.h>
int main(int argc,char* argv[])
{
	CRTSPStream rtspSender;
	bool bRet = rtspSender.Init();
	rtspSender.SendH264File(argv[1]);
	
sleep(10000);
	//system("pause");  
}

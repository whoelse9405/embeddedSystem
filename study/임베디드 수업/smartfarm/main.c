#include <stdio.h>			
#include <sys/socket.h>	
#include <arpa/inet.h>		
#include <stdlib.h>		
#include <string.h>			
#include <unistd.h>		
#include <sys/time.h>
#include <sys/types.h>

#define MAXCLIENT   10
#define MAXPENDING 	5

#define TYPE_TRUE   1
#define TYPE_FALSE  0
#define ERROR_PARSE -1
#define ERROR_NULL  -2

enum MESSAGE_TYPE{farm, secret};

typedef struct _Message
{
    enum MESSAGE_TYPE _datatype;
    char *       _data;
}Message;

typedef struct _TInfo
{
	unsigned int	uiUser;
	int				iSock;
	pthread_t		tID;
} TInfo;

void *ClientRecv(void *);
int parse_data(char * data, Message * msg);

unsigned int		uiUser;
TInfo *             stpLink[MAXCLIENT];
pthread_mutex_t		MLock;

int main(int iArg, char *cpArg[])
{
	int servSock; 
	TInfo stTempInfo;
	struct sockaddr_in echoServAddr;
	struct sockaddr_in echoClntAddr;
	unsigned short echoServPort;
	unsigned int clntLen;
	int iRet;
	int iCnt;
	int iCnt2;
	unsigned char ucBuff[500];

	if (1 == iArg)
	{
		echoServPort = 9999;
	}
	else if (2 == iArg)
	{
		echoServPort = atoi(cpArg[1]);
	}
		
	servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(0 > servSock)
	{
		printf("socket() failed");

		return 0;
	}

	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	echoServAddr.sin_port = htons(echoServPort);

	iRet = bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
	if(0 > iRet)
	{
		close(servSock);
		printf("bind() failed");

		return 0;
	}

	iRet = listen(servSock, MAXPENDING);
	if(0 > iRet)
	{
		close(servSock);
		printf("listen() failed");

		return 0;
	}

	clntLen = sizeof(echoClntAddr);

	uiUser = 0;
	pthread_mutex_init(&MLock, NULL);			
	while(1)
	{
		stTempInfo.iSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen);
		if(0 > stTempInfo.iSock)			
		{
			printf("accept() failed");
			continue;	
		}	
		printf("Handling client ip : %s\n", inet_ntoa(echoClntAddr.sin_addr));
		printf("Handling client port : %d\n", ntohs(echoClntAddr.sin_port));
		printf("Handling client socket number : %d\n", stTempInfo.iSock);
		pthread_mutex_lock (&MLock);		
		stTempInfo.uiUser = uiUser;
		pthread_create (&stTempInfo.tID, 0, ClientRecv, &stTempInfo);
		++uiUser;								
		pthread_mutex_unlock (&MLock);

		while (0 != stTempInfo.iSock);			
		printf("현재 접속자 수 : %d\n", uiUser);
	}

	close(servSock);

	return 0;
}

int parse_data(char * data, Message * msg)
{
    //1. valified 
    if(0)
        return ERROR_PARSE;
    else
    {
        //2. parsing
        return TYPE_TRUE;
    }
}

void *ClientRecv(void *vp)						
{
	unsigned char	ucBuff[500];
	unsigned char	ucSBuff[500];
	unsigned int	uiCnt;
	int				iRet;
	TInfo			stMyInfo = *((TInfo *)vp);

	stpLink[stMyInfo.uiUser] = &stMyInfo;
	((TInfo *)vp)->iSock = 0;					
												
	while(1)
	{
		iRet = read (stMyInfo.iSock, ucBuff, 500);
		if (1 > iRet)
		{
			break;
		}
		ucBuff[iRet - 1] = 0;					
		printf ("[%dSock][MyUserNum:%d]:[%s]\n", stMyInfo.iSock, stMyInfo.uiUser, ucBuff);
		if ('$' == ucBuff[0])
		{
			break;
		}
		iRet = sprintf (ucSBuff, "[%dSock][MyUserNum:%d]:[%s]\n", stMyInfo.iSock, stMyInfo.uiUser, ucBuff);

        //0. parsing data

        //3. sensor access 

        //4. apply

        //send by 1 
        write (stMyInfo.iSock, ucSBuff, iRet);

#ifdef USE_BROADCAST
		for (uiCnt=0 ; uiUser>uiCnt; ++uiCnt)
		{
			if (&stMyInfo == stpLink[uiCnt])
			{
				continue;
			}
			write (stpLink[uiCnt]->iSock, ucSBuff, iRet);
		}
#endif
	}
	pthread_mutex_lock (&MLock);			
	--uiUser;	
	stpLink[stMyInfo.uiUser] = stpLink[uiUser];
	stpLink[stMyInfo.uiUser]->uiUser = stMyInfo.uiUser;
	pthread_mutex_unlock (&MLock);
	close(stMyInfo.iSock);
	return 0;
}


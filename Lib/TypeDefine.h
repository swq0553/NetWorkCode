#pragma once
#include <WinSock2.h>
#include <Windows.h>

#pragma comment (lib ,"Ws2_32.lib")

//namespace TheOne
//{
#define  MAX_CLIENT_COUNT              8000
#define  MAX_IOCP_BUF_SIZE            (8 * 1024)
#define  SHUTDOWN_FLAG                 0XFFFFFFFF
#define  DISCONNECT_FLAG               0XFFFFFFFE
#define  MAX_WORKTHREAD_SIZE           16
#define  HASH_IPITEM_BUCKETS           2047
#define  MAX_CHECK_TIMEOUT            (30 * 1000)  //�̵߳ļ����
#define  MAX_ACTIVE_TIMEOUT           (30 * 60)    //��ʱ�Ͽ���ʱ��
#define  MAX_FREE_TIMEOUT             (30000 * 60)     //�Ͽ��ͷŵ�ʱ��
#define  CSocket        int;

enum CSocketOperator { 
	 seInitIOPort,
	 seInitSocket,
	 seConnect,
	 seDisConnect,
	 seListen,
	 seAccept,
	 seWrite,
	 seRead
} ;

typedef struct _IOCPSOCKET{
	int socket;
	char ip[16];
	int  port;
	_IOCPSOCKET()
	{
		socket = INVALID_SOCKET;
		memset(ip ,0 ,sizeof(ip));
		port = 0;
	}
}IOCPSOCKET,*pIOCPSOCKET;


typedef struct _Block
{
	OVERLAPPED  overlapped;                            //�ص�
	WSABUF      wsaBuffer;                             //ϵͳ����
	int         socket_state;                          //���SOCKET��д
	TCHAR        szBuffer[MAX_IOCP_BUF_SIZE+1];         //�û�����
}Block , *pBlock;

typedef struct _SendQueueNode
{
	char * szbuf;
	int iBufLen;
	int iStartPos;
	bool bLocalbuf;
	_SendQueueNode * Next;
	_SendQueueNode(){
       szbuf = NULL;
	   iBufLen = 0;
	   iStartPos = 0 ;
	   bLocalbuf = false;
	   Next = NULL;
	}
}CSendQueueNode , *pSendQueueNode;
//};
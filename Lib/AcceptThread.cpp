#include "StdAfx.h"
#include "AcceptThread.h"
#include "ServerSocket.h"
namespace TheOne
{
	CAcceptThread::CAcceptThread( CServerSocket * pParentSocket )
	{
		m_serversocket = pParentSocket;
		Start(false);
	}

	CAcceptThread::~CAcceptThread(void)
	{
		if(m_serversocket != NULL)
		{
			IOCPSOCKET * socketinfo = m_serversocket->GetIOCPSocket();
			if ((socketinfo != NULL) && (socketinfo->socket != INVALID_SOCKET))
			{
				closesocket(socketinfo->socket);
				socketinfo->socket = INVALID_SOCKET;

			}
		}
	}

	void CAcceptThread::Execute( void )
	{
		int nPort ,Addrlen;
		sockaddr_in toAddr;
	    SOCKET s,ListenSocket;
		char IpAddress[16]={0};
		pIOCPSOCKET socketinfo = m_serversocket->GetIOCPSocket();
		if ((socketinfo ==NULL) ||(socketinfo->socket == INVALID_SOCKET)) return;
		ListenSocket = socketinfo->socket;
		if(listen(ListenSocket,5)!=0)
		{
			return;
		}
		while (!m_bTerminated)
		{
			try
			{
				memset(&toAddr,0,sizeof(sockaddr_in));
				Addrlen = sizeof(sockaddr_in);
				s = WSAAccept(ListenSocket,(sockaddr *)&toAddr,&Addrlen,NULL,(DWORD)(m_serversocket));
				if(m_bTerminated) 
				{
					break;
				}
				if(s != INVALID_SOCKET)
				{
					char* pAddr = inet_ntoa(toAddr.sin_addr);
					if(pAddr != NULL)
					{
						memset(IpAddress,0,sizeof(IpAddress));
						lstrcpy(IpAddress,pAddr);
						nPort = ntohs(toAddr.sin_port);
						m_serversocket->SocketAccept(s,IpAddress,nPort);

					}
					else
					{
						m_serversocket->SocketErrorEvent(NULL,GetLastError());

					}
				}
				else
					m_serversocket->SocketErrorEvent(NULL,GetLastError());
			}
			catch (...)
			{
				
			}
		}

	}

}



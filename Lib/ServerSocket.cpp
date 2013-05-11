#include "StdAfx.h"
#include "ServerSocket.h"
#include "AcceptThread.h"
namespace TheOne{
	CServerSocket::CServerSocket( void )
	{
		m_wCurrentHandle = 1000;
		SYSTEM_INFO sysinfo;
		m_AcceptThread = NULL;
		GetSystemInfo(&sysinfo);
		m_iWorkThreadCount = sysinfo.dwNumberOfProcessors ;
		if(m_iWorkThreadCount >= MAX_WORKTHREAD_SIZE)
		{
			m_iWorkThreadCount = MAX_WORKTHREAD_SIZE ;
		}
		m_WorkThread = new CWorkThread*[m_iWorkThreadCount];
		
		for (int i = 0 ;i< m_iWorkThreadCount;i++)
		{
			m_WorkThread[i]=NULL;
		}
		InitializeCriticalSection(&m_Lock);
		m_dwTimeOutCheckTick = GetTickCount();
		m_SessionMap.clear();
	}

	CServerSocket::~CServerSocket(void)
	{
		
		Close();
		delete m_WorkThread;
		DeleteCriticalSection(&m_Lock);
	}

	void CServerSocket::FreeCorpseClient()
	{
		DWORD Tick, DTick;
		Tick =GetTickCount();
		DTick = Tick - m_dwTimeOutCheckTick;
		CCustomClientSocket * ClientSocket;
		if(DTick > MAX_CHECK_TIMEOUT)
		{
			m_dwTimeOutCheckTick = Tick;
			Tick = DTick /1000;
			m_FreeClientPool.FreeTimeOutClient(Tick);
			int iCount=m_ActiveClientSocketList.size();
			for(int i=0;i<iCount;i++)
			{
				ClientSocket=m_ActiveClientSocketList[i];
				if (ClientSocket!=NULL)
				{      
					ClientSocket->m_CurrentTick +=Tick;
					if(ClientSocket->m_ActiveTick==0)
					{
						ClientSocket->m_ActiveTick =ClientSocket->m_CurrentTick;
					}else if(ClientSocket->m_CurrentTick -ClientSocket->m_ActiveTick> MAX_ACTIVE_TIMEOUT)
					{
						ClientSocket->m_ActiveTick =ClientSocket->m_CurrentTick;
						ClientSocket->ForceClose();
					}
				}
			}
		}
	}

	CCustomClientSocket * CServerSocket::ValueOf( const DWORD Key )
	{
		ClientListMapIter iter = m_SessionMap.find(Key);
		if(iter != m_SessionMap.end())
		{
			return iter->second;
		}
		else
			return NULL;
	}

	void CServerSocket::SocketAccept( SOCKET ssocket,char* ip,int port )
	{
		CCustomClientSocket * ClientSocket =NULL;
		Lock();
		FreeCorpseClient();
		ClientSocket = m_FreeClientPool.DeQueue();
		if (ClientSocket != NULL)
		{
			ClientSocket = CreateClientSocket();
			if(ClientSocket == NULL)
			{
				ClientSocket = new CCustomClientSocket;

			}
		}
		ClientSocket->SetSocketHandle(m_wCurrentHandle);
		ClientSocket->setRemoteAddress(ip);
		ClientSocket->setRemotePort(port);
		ClientSocket->SetIOCPHandle(m_hIOCP);
		ClientSocket->SetSocket(ssocket);

		ClientSocket->OnInitialize();
		m_ActiveClientSocketList.push_back(ClientSocket);
		m_iClientCount ++;
		m_SessionMap[m_wCurrentHandle]=ClientSocket;
		if(ClientSocket->GetSocket() != INVALID_SOCKET)
		{
			if(CreateIoCompletionPort((HANDLE)ssocket,m_hIOCP,DWORD(ClientSocket),0)==0)
			{
				OnError((CCustomSocket *)ClientSocket,GetLastError());
				ClientSocket->ForceClose();
			}
			else
			{
				OnConnect((CCustomSocket *)ClientSocket);
				if (ClientSocket->GetSocket() != INVALID_SOCKET)
				{
					ClientSocket->PrepareRecv(ClientSocket->GetBlock(true));
				}
			}
		}
		ClientSocket = NULL;
		do {
			m_wCurrentHandle++;
			if(m_wCurrentHandle < 1000)
			{
				m_wCurrentHandle = 1000;
			}}while (m_SessionMap.find(m_wCurrentHandle)!= m_SessionMap.end());
	   Unlock();
		
	}

	void CServerSocket::Close()
	{
		Lock();
		CCustomClientSocket * ClientSocket;
		SetActive(false);

		for(int i =0 ;i< (int)m_ActiveClientSocketList.size();i++)
		{
			ClientSocket = m_ActiveClientSocketList[i];
			if(ClientSocket !=NULL)
			{
				FreeClient(ClientSocket);
			}
		}
		Unlock();
	  m_FreeClientPool.FreeTimeOutClient(MAX_FREE_TIMEOUT);

	}

	void CServerSocket::FreeClient( CCustomClientSocket * ClientSocket )
	{
		Lock();
		m_SessionMap[m_wCurrentHandle] = NULL;
		for(ClientListIter iter = m_ActiveClientSocketList.begin();iter != m_ActiveClientSocketList.end();iter++)
		{
			if (*iter = ClientSocket)
			{
				m_ActiveClientSocketList.erase(iter);
				m_iClientCount--;
				m_FreeClientPool.Append(ClientSocket);
				if(ClientSocket->GetSocket()!=INVALID_SOCKET)
				{
					ClientSocket->ForceClose();

				}
				OnDisConnect((CCustomSocket*)ClientSocket);
				ClientSocket->Finalize();
				return;
			}
		}
	  Unlock();
	}


	void CServerSocket::SetActive( bool value )
	{
		if (value)
		{
			if((m_IOCPSOCKET.ip == "")||(m_IOCPSOCKET.port == 0)||(m_IOCPSOCKET.socket !=INVALID_SOCKET))
				return;
			int i = 0;
			for ( i = m_iWorkThreadCount ;i >=0 ;i--)
			{
				if(m_WorkThread[i]!=NULL)
				{
				   m_WorkThread[i]->Terminate();
				   delete m_WorkThread[i];
				   m_WorkThread[i]=NULL;
				}
			}
			m_WorkThread[0]= new CWorkThread(this,true);

		}
		else
		{
			PostQueuedCompletionStatus(m_hIOCP,0,0,LPOVERLAPPED(SHUTDOWN_FLAG));
			int i = 0;
			for ( i = m_iWorkThreadCount-1 ;i >=0 ;i--)
			{
				if(m_WorkThread[i]!=NULL)
				{
				   m_WorkThread[i]->Terminate();
				   delete m_WorkThread[i];
				   m_WorkThread[i]=NULL;
				}
			}
		}
	}

	bool CServerSocket::ActiveStart()
	{
		HANDLE hIOCPHandle;
		sockaddr_in addr;
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
		if(m_hIOCP!=0)
		{
			m_IOCPSOCKET.socket = WSASocket(PF_INET,SOCK_STREAM,IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);
			if(m_IOCPSOCKET.socket != INVALID_SOCKET)
			{
				memset(&addr,0,sizeof(sockaddr_in));
				addr.sin_family = PF_INET;
				if(strcmp(m_IOCPSOCKET.ip,"0.0.0.0")==0)
				{
					addr.sin_addr.S_un.S_addr = INADDR_ANY;

				}
				else
				{
					addr.sin_addr.S_un.S_addr = inet_addr(m_IOCPSOCKET.ip);
				}
				WSAHtons(m_IOCPSOCKET.socket,m_IOCPSOCKET.port,&addr.sin_port);
				//°ó¶¨Socket
				if(bind(m_IOCPSOCKET.socket,(sockaddr*)&addr,sizeof(sockaddr_in))==0)
				{
					m_AcceptThread = new CAcceptThread(this);
					m_AcceptThread->Resume();
					for (int i = 0 ;i<= m_iWorkThreadCount ;i++)
					{
						m_WorkThread[i] = new CWorkThread(this,false);
					}
					return true;

				}
				else
				{
					this->OnError(NULL,GetLastError());
					closesocket(m_IOCPSOCKET.socket);
					m_IOCPSOCKET.socket = INVALID_SOCKET;
				}
			}//endif
			else
			{
				OnError(NULL,WSAGetLastError());			
				hIOCPHandle = m_hIOCP;
				m_hIOCP = 0 ;
				CloseHandle(m_hIOCP);
			}
		}
		else
			OnError(NULL,GetLastError());
			return false;
	}

	void CServerSocket::ActiveStop()
	{
		CCustomClientSocket * client = NULL;
		if(m_AcceptThread != NULL)
		{
			m_AcceptThread->Terminate();
			delete m_AcceptThread;
			m_AcceptThread = NULL;
		}
		for (int i = m_ActiveClientSocketList.size()-1 ;i>=0 ;i--)

		{
			client = m_ActiveClientSocketList[i];
			if(client!=NULL)
			{
				client->ForceClose();
			}
		}
		if(m_hIOCP != 0 )
		{
			HANDLE hIOCP = m_hIOCP;
			m_hIOCP = 0 ;
			CloseHandle(hIOCP);
		}

	}



	CCustomClientPool::CCustomClientPool( void )
	{
		m_FFirstQueueNode = NULL;
		m_FLastQueueNode = NULL;
		m_iCount = 0 ;


	}

	CCustomClientPool::~CCustomClientPool( void )
	{
		Clear();
	}

	bool CCustomClientPool::IsTimeOut( CCustomClientSocket * pSocket,DWORD tick )
	{
		bool ret = false;
		pSocket->m_CurrentTick += tick;
		if(pSocket->m_ActiveTick == 0 )
		{
			pSocket->m_ActiveTick = pSocket->m_CurrentTick;
		}else
		if(pSocket->m_CurrentTick - pSocket->m_ActiveTick >= MAX_FREE_TIMEOUT)
		{
			delete pSocket;
			ret = true;

		}
		return ret;
	}

	void CCustomClientPool::Append( CCustomClientSocket * pClientSocket )
	{
		pCustomClientSocketNode NewNode;
		NewNode = new CustomClientSocketNode;
		NewNode->ClientSocket = pClientSocket;
		NewNode->Next = NULL;

		if (m_FLastQueueNode != NULL)
		{
			m_FLastQueueNode->Next = NewNode;

		}else
		if(m_FFirstQueueNode == NULL)
		{
			m_FFirstQueueNode->Next = NewNode;

		}
		m_FLastQueueNode = NewNode;
		m_iCount ++;

	}

	CCustomClientSocket * CCustomClientPool::DeQueue( void )
	{
		CCustomClientSocket * ClientSocket;
		ClientSocket = NULL;
		pCustomClientSocketNode p;

		if(m_FFirstQueueNode != NULL)
		{
			p = m_FFirstQueueNode;
			ClientSocket = p->ClientSocket;
			m_FFirstQueueNode = p->Next;
			delete p ;
			m_iCount --;
			if (m_FFirstQueueNode == NULL)
			{
				m_FLastQueueNode = NULL;
			}
		}
		return ClientSocket;

	}

	void CCustomClientPool::Clear()
	{
		pCustomClientSocketNode p ;
		while (m_FFirstQueueNode != NULL)
		{
			p = m_FFirstQueueNode;
			m_FFirstQueueNode = p->Next;
			delete p;
		}
		m_FFirstQueueNode = NULL;
		m_FLastQueueNode = NULL;
		m_iCount = 0 ;
	}

	void CCustomClientPool::FreeTimeOutClient( DWORD tick )
	{
		pCustomClientSocketNode p;
		while(m_FFirstQueueNode != NULL)
		{
			if(this->IsTimeOut(m_FFirstQueueNode->ClientSocket,tick))
			{
				m_iCount--;
				p = m_FFirstQueueNode;
				m_FFirstQueueNode = p->Next;
				delete p;
			}else
				break;
		}
		if(m_FFirstQueueNode == NULL)
			m_FLastQueueNode = NULL;

	}

}

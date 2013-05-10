#include "StdAfx.h"
#include "WorkThread.h"
#include<Windows.h>

namespace TheOne
{


	CWorkThread::CWorkThread( CCustomSocket * parent ,bool boMaster,bool boServer /*= true*/ )
	{
		m_ParentSocket = parent;
		m_Master = boMaster;
		m_BoServer = boServer;
		CThread::Start(false);

	}

	CWorkThread::~CWorkThread( void )
	{

	}

	void CWorkThread::Execute( void )
	{
		CCustomClientSocket * ClientSocket = NULL;
		Block * pBlock = NULL;
		DWORD dwBytesXfred= 0 ;
		BOOL  ret = false;
		if ((m_Master)&&(!m_ParentSocket->Initialize()))
		{
			Sleep(2000);
			m_ParentSocket->Finalization();
			return;
		}
		while (!m_bTerminated)
		{
			try
			{
				if(!m_ParentSocket->GetActive())
				{
					break;

				}
				ret= GetQueuedCompletionStatus(m_ParentSocket->GetIOCPHandle(),(LPDWORD)(&dwBytesXfred),(PDWORD_PTR)(&ClientSocket),(LPOVERLAPPED*)(&pBlock),INFINITE);

				if((DWORD)pBlock ==SHUTDOWN_FLAG)
				{
					Terminate();
				}
				if(m_bTerminated)
				{
					if(ClientSocket != NULL)
					{
						ClientSocket->ForceClose();
					}
					break;
				}
				if((DWORD(pBlock) == DISCONNECT_FLAG)&&(ClientSocket !=NULL))
				{
					m_ParentSocket->ReleaseClient(ClientSocket);
					continue;

				}
				if((!ret)||(dwBytesXfred ==0))
				{
					if(ClientSocket != NULL)
					{
						ClientSocket->ForceClose();
					}
					continue;;
				}
				if ((ClientSocket!=NULL)&&(pBlock != NULL))
				{
					ClientSocket->m_ActiveTick = 0 ;
					switch (pBlock->socket_state)
					{
					case seWrite:
						ClientSocket->DoClientWrite(pBlock,dwBytesXfred);
						break;
					case seRead:
						if(m_BoServer)
						{
							ClientSocket->DoClientRead(pBlock,dwBytesXfred);
						}
						else
						{
							if(dwBytesXfred > 0)
							{
								pBlock->szBuffer[dwBytesXfred]=0;
								try
								{
									m_ParentSocket->DoRead((CCustomSocket*)ClientSocket,(char*)pBlock->szBuffer,dwBytesXfred);

								}
								catch (...)
								{
									//::OutputDebugString("DoRead Error");	
								}
							
								if(ClientSocket->GetSocket() != INVALID_SOCKET)
								{
									ClientSocket->PrepareRecv(pBlock);
								}
							}
						}
						break;
					default:
						m_ParentSocket->SocketErrorEvent((CCustomSocket*)ClientSocket,GetLastError());
					}
				}
			}
			catch(...)
			{
				m_ParentSocket->SocketErrorEvent((CCustomSocket*)ClientSocket,GetLastError());
				continue;
			}
		}
		PostQueuedCompletionStatus(m_ParentSocket->GetIOCPHandle(),0,0,LPOVERLAPPED(SHUTDOWN_FLAG));
		if(m_Master)
		{
			m_ParentSocket->Finalization();
		}
		m_ParentSocket = NULL;
		if (pBlock != NULL )
		{
			delete pBlock;
		}
		if(ClientSocket != NULL)
		{
			delete ClientSocket;
		}

	}

}


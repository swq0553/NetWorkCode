#include "StdAfx.h"
#include "CustomClientSocket.h"
namespace TheOne
{
	CCustomClientSocket::CCustomClientSocket(void)
	{
		m_iSocket = INVALID_SOCKET;
		memset(&m_BRecvBlock,0,sizeof(m_BRecvBlock));
		memset(&m_BSendBlock,0,sizeof(m_BSendBlock));
		m_bSending = false;
		m_FirstNode = NULL;
		m_EndNode = NULL;
		m_iTitleBufLen = 0;
		m_dwTotalSendDataCount = 0;
		m_wSocket = 0;
		m_hIocp = 0 ;
		InitializeCriticalSection(&m_cLock);

	}

	CCustomClientSocket::~CCustomClientSocket(void)
	{
		DeleteCriticalSection(&m_cLock);
		Close();

	}

	void CCustomClientSocket::DoClientRead( pBlock bBlock ,int Transfered )
	{
		if (Transfered > 0) 
		{
#ifdef DEBUG
		//	::OutputDebugStr("DoClientRead \r\n");
#endif
			bBlock->szBuffer[Transfered] = 0 ;
			__try
			{
				onRead((char*)bBlock->szBuffer,Transfered);
			}
			__finally
			{
				if (m_iSocket != INVALID_SOCKET)
				{
					PrepareRecv(bBlock);
				}
			}
		}

	}

	void CCustomClientSocket::DoClientWrite( pBlock bBlock ,int Transfered )
	{
#ifdef DEBUG

#endif
		int Sendlen;
		Sendlen = bBlock->wsaBuffer.len - Transfered;
		if (Sendlen > 0)
		{
			memmove(&bBlock->szBuffer[0],&bBlock->szBuffer[Transfered],Sendlen);
		}
		PrepareSend(bBlock,Sendlen);
	}

	void CCustomClientSocket::PrepareRecv( pBlock bBlock )
	{
		DWORD flages,Transfered;
		try
		{
			bBlock->socket_state = seRead;
			bBlock->wsaBuffer.len = MAX_IOCP_BUF_SIZE;
			bBlock->wsaBuffer.buf = (char*)bBlock->szBuffer;
			memset(bBlock->szBuffer,0,sizeof(bBlock->wsaBuffer.len));
			flages = 0;
			memset(&bBlock->overlapped , 0 ,sizeof(bBlock->overlapped));
			if((m_iSocket == INVALID_SOCKET) || 
			   (WSARecv(m_iSocket,LPWSABUF(bBlock->szBuffer),1,&Transfered,&flages,(LPWSAOVERLAPPED)bBlock,NULL)== SOCKET_ERROR))
			{
				int Errorcode = WSAGetLastError();
				if (Errorcode != ERROR_IO_PENDING)
				{
					onError(Errorcode);
					Close();
				}
			}

		}
		catch(...)
		{
			onError(GetLastError());
		}
	}

	void CCustomClientSocket::PrepareSend( pBlock bBlock ,int iSendlen )
	{
		int iRemainLen,iDataLen,iErrorCode;
		DWORD Transfered;
		pSendQueueNode nNode;
		EnterCriticalSection(&m_cLock);
		if (iSendlen < 0 )
		{
			iSendlen = 0 ;

		}
		m_bSending = false;
		while(m_FirstNode != NULL)
		{
			nNode = m_FirstNode;
			iRemainLen = MAX_IOCP_BUF_SIZE;
			iDataLen = nNode->iBufLen - nNode->iStartPos;
			if(iDataLen > iRemainLen)
			{
				memmove(&bBlock->szBuffer[iSendlen],&nNode->szbuf[nNode->iStartPos],iRemainLen);
				iSendlen = MAX_IOCP_BUF_SIZE;
				nNode->iStartPos  += iRemainLen;
				break;
			}else
			{
				memmove(&bBlock->szBuffer[iSendlen],&nNode->szbuf[nNode->iStartPos],iDataLen);
				iSendlen = iDataLen;
				m_FirstNode = nNode->Next;
				if(m_FirstNode == NULL)
				{
					m_EndNode = NULL;
				}
				if(nNode->bLocalbuf)
				{
					//释放本地内存
				}
				else
				{
					free(nNode->szbuf);
				}
				delete(nNode);
			}
		}
		LeaveCriticalSection(&m_cLock);
		m_iTitleBufLen -= iSendlen;
		if(iSendlen > 0 )
		{
			m_bSending = true;
			bBlock->socket_state = seWrite;
			bBlock->wsaBuffer.len = iSendlen;
			bBlock->wsaBuffer.buf = (char*)bBlock->szBuffer;
			memset(&bBlock->overlapped ,0 ,sizeof(bBlock->overlapped));
			if ((m_iSocket != INVALID_SOCKET)||
				(WSASend(m_iSocket,LPWSABUF(&bBlock->wsaBuffer),1,&Transfered ,0,(LPWSAOVERLAPPED)bBlock ,NULL) == SOCKET_ERROR))
			{
				iErrorCode = WSAGetLastError();
				if (iErrorCode != ERROR_IO_PENDING)
				{
					onError(iErrorCode);
					Close();
				}
			}
		}
	}

	int CCustomClientSocket::ForceClose()
	{
		int iRetCode = 0 ;
		if(m_iSocket != INVALID_SOCKET)
		{
			__try
			{
				iRetCode = closesocket(m_iSocket);
			}__finally
			{
				m_iSocket = INVALID_SOCKET;
				ClearSendBuffer();
				memset(&m_BRecvBlock,0,sizeof(Block));
				PostQueuedCompletionStatus(m_hIocp,0,DWORD(this),LPOVERLAPPED(DISCONNECT_FLAG));
			}
		}
		return iRetCode;
	}

	void CCustomClientSocket::ClearSendBuffer()
	{
		pSendQueueNode nNode;
		EnterCriticalSection(&m_cLock);
		while (m_FirstNode != NULL)
		{
			nNode = m_FirstNode;
			m_FirstNode = nNode->Next;
			if(nNode->bLocalbuf)
			{
				//管理器释放内存
			}
			else
			{
				free(nNode->szbuf);
			}
			delete nNode;
		}
		m_EndNode = NULL;
		m_bSending = false;
		m_iTitleBufLen = 0;
		LeaveCriticalSection(&m_cLock);
	}

	void CCustomClientSocket::onErrorEvent( int ErrorCode )
	{
		if (ErrorCode != ERROR_IO_PENDING)
		{
			onError(ErrorCode);
		}
	}

	void CCustomClientSocket::OnInitialize()
	{
		m_bSending  = false;
		m_CurrentTick = 0;
		m_ActiveTick = 0 ;
		m_iTitleBufLen = 0;
		m_dwTotalSendDataCount = 0;
		Initialize();

	}

	void CCustomClientSocket::OnFinalize()
	{
		m_bSending = false;
		Finalize();
	}

	void CCustomClientSocket::Close()
	{
		if(ForceClose() != 0)
		{
			onErrorEvent(WSAGetLastError());
		}
	}

	int CCustomClientSocket::SendBuf( const char * szBuf ,int icount ,bool FreeAfterSender /*= false*/ )
	{
		if ((icount <= 0 )||(m_iSocket != INVALID_SOCKET))
		{
			if(FreeAfterSender)
			{
				free((char*)szBuf);
			}
			return 0 ;

		}
		pSendQueueNode pNode = NULL;
		pNode = new CSendQueueNode;
		if(pNode == NULL)
		{
			if(FreeAfterSender)
			{
				free((char*)szBuf);
			}
			return 0 ;
		}
		pNode->bLocalbuf = false;
		pNode->iStartPos = 0;
		pNode->iBufLen = icount;
		if(!FreeAfterSender)
		{
			//这里是用内存管理器给NODE分配内存*pNode->szbuf = (char*)*/
		}
		else
		{
			pNode->szbuf = (char *) szBuf;
		}
		EnterCriticalSection(&m_cLock);
		m_iTitleBufLen += icount;
		m_dwTotalSendDataCount += icount;
		if(m_EndNode !=NULL)
		{
			m_EndNode->Next = pNode;

		}
		if(m_FirstNode ==NULL)
		{
			m_FirstNode =pNode;
		}
		m_EndNode = pNode;
		LeaveCriticalSection(&m_cLock);
		if(!m_bSending)
		{
			PrepareSend(&m_BSendBlock,0);
		}
		return icount;

	}

	int CCustomClientSocket::SendText( const string strdata )
	{
		int strlen = strdata.length();
		if(strlen > 0)
		{
			return SendBuf(strdata.c_str(),strdata.length(),false);
		}
		else
			return 0;
	}

	pBlock CCustomClientSocket::GetBlock( bool bRead )
	{
		return &(this->m_BSendBlock);
	
	}




}


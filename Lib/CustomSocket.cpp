#include "StdAfx.h"
#include "CustomSocket.h"

namespace TheOne
{
	CCustomSocket::CCustomSocket(void)
	{
	   WORD wVersionRequested = 0x202;
	   WSAStartup(wVersionRequested,&m_data);
	}

	CCustomSocket::~CCustomSocket(void)
	{
		SetActive(false);
		WSACleanup();
	}

	void CCustomSocket::SetServerIp( const string StrIP )
	{
       if (m_IOCPSOCKET.socket == INVALID_SOCKET)
		   memmove(m_IOCPSOCKET.ip,StrIP.c_str(),StrIP.length());
	}

}


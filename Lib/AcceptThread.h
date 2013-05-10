#pragma once
#include "Thread.h"
#include "TypeDefine.h"

namespace TheOne
{
	class CServerSocket;
	class CAcceptThread :
		public CThread
	{
	public:
		//CAcceptThread(void);
		CAcceptThread( CServerSocket * pParentSocket);
		~CAcceptThread(void);

	private:
		CServerSocket * m_serversocket;

	protected:
		virtual void Execute(void);
	
	};

}

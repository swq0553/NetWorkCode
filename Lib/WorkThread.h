#pragma once

#include "Thread.h"
#include "CustomSocket.h"
#include "CustomClientSocket.h"

namespace TheOne
{

	class CWorkThread :
		public CThread
	{
	public:
		//CWorkThread(void);
		~CWorkThread(void);
	private:
		bool m_Master;
		bool m_BoServer;
		CCustomSocket * m_ParentSocket;
	protected:
		virtual void Execute(void);
	public:
		CWorkThread(CCustomSocket * parent ,bool boMaster,bool boServer = true);
	};

}

#pragma once
#include "TypeDefine.h"
#include "CustomSocket.h"
#include "CustomClientSocket.h"
#include "AcceptThread.h"
#include "WorkThread.h"
#include <hash_map>

using namespace std;
using namespace stdext;

namespace TheOne{

	typedef stdext::hash_map<WORD,CCustomClientSocket*> ClientListMap;
	typedef stdext::hash_map<WORD,CCustomClientSocket*>::iterator ClientListMapIter;
	typedef std::vector<CCustomClientSocket*> ClientList;
	typedef std::vector<CCustomClientSocket*>::iterator ClientListIter;

typedef struct _CustomClientSocketNode
{
	CCustomClientSocket * ClientSocket;
	_CustomClientSocketNode  * Next;
}CustomClientSocketNode,*pCustomClientSocketNode;


class CCustomClientPool
{
	public:
		CCustomClientPool(void);
		virtual ~CCustomClientPool(void);
	private:
		pCustomClientSocketNode m_FFirstQueueNode;
		pCustomClientSocketNode m_FLastQueueNode;
		int m_iCount;
		bool IsTimeOut(CCustomClientSocket * pSocket,DWORD tick);

	public:
		void Append(CCustomClientSocket * pClientSocket);
		CCustomClientSocket * DeQueue(void);
		void Clear();
		void FreeTimeOutClient(DWORD tick);
		int GetCount(){return m_iCount;}



};
class CServerSocket :
	public CCustomSocket
{
public:
	CServerSocket(void);
	~CServerSocket(void);
private:
	CRITICAL_SECTION m_Lock;
	ClientList m_ActiveClientSocketList;
	int m_iClientCount;
	int m_iHashHandleCount;
	//客户端SOCKET池
	CCustomClientPool m_FreeClientPool;

	DWORD m_dwTimeOutCheckTick;
	CAcceptThread* m_AcceptThread;
	CWorkThread** m_WorkThread;
	WORD m_wCurrentHandle;
	int m_iWorkThreadCount;
	int GetPoolCount(){return m_FreeClientPool.GetCount();};//这个还需要修改。。
	virtual void SetActive(bool value);
	void FreeCorpseClient();
	virtual void FreeClient(CCustomClientSocket * ClientSocket);
	bool ActiveStart();
	void ActiveStop();
	bool DisposeTimeoutClient(CCustomClientSocket * ClientSocket,DWORD param);
	bool IsValidAddress(const string IP ,int iIP,int iPort);
	
	
	
	virtual bool ChcekAddressIp(const string strIP){return true;};
	virtual CCustomClientSocket * CreateClientSocket(){return new CCustomClientSocket;}


protected:
	ClientListMap m_SessionMap;
public:
	void Close();
	void SocketAccept(SOCKET ssocket,char* ip,int port);
	void Lock(){EnterCriticalSection(&m_Lock);};
	void Unlock(){LeaveCriticalSection(&m_Lock);};
	CCustomClientSocket * ValueOf(const DWORD Key);
	ClientList GetActiveSocket() const {return m_ActiveClientSocketList;};


};
}
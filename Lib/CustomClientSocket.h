#pragma once
#include "CustomSocket.h"
#include "TypeDefine.h"
#include <string>

using namespace std;
using namespace stdext;

namespace TheOne
{
	class CCustomClientSocket
	{
	public:
		CCustomClientSocket(void);
		~CCustomClientSocket(void);

	private:
		WORD   m_wSocket;
		HANDLE m_hIocp;
		int    m_iSocket;
		string m_strRemoteAddress;
		int    m_nRemotePort;
		Block  m_BSendBlock;
		Block  m_BRecvBlock;
		int    m_iTitleBufLen;
		CRITICAL_SECTION m_cLock;
		bool   m_bSending;
		DWORD  m_dwTotalSendDataCount;
		pSendQueueNode m_FirstNode;
		pSendQueueNode m_EndNode;

		void ClearSendBuffer();

	private:
		//连接
		virtual void onConnect(bool bSuccess){};
		//断开连接
		virtual void onDisConnect(){};
		//数据到达
		virtual void onRead(const char* pBuf,int nlen){};
		//发生错误
		virtual void onError(int nErrorcode){};
	
	public:
		DWORD m_ActiveTick;
		DWORD m_CurrentTick;
		//初始化
		virtual void Initialize(){};
		//释放清理
		virtual void Finalize(){};
		void PrepareRecv(pBlock bBlock);
		void PrepareSend(pBlock bBlock ,int iSendlen);
		void DoClientRead(pBlock bBlock ,int Transfered);
		void DoClientWrite(pBlock bBlock ,int Transfered);
		void onErrorEvent(int ErrorCode);
		void OnInitialize();
		void OnFinalize();
		void Close();
		int  ForceClose();
		int  SendBuf(const char * szBuf ,int icount ,bool FreeAfterSender = false);
		int  SendText(const string strdata);
		void setRemoteAddress(const string strIp) {m_strRemoteAddress = strIp;};
		string getRemoteAddress(){return m_strRemoteAddress;};
		void setRemotePort(int nport){ m_nRemotePort = nport;};
		int getRemotePort(){return m_nRemotePort;};
		int getSendBuflen(){return m_iTitleBufLen;};
		DWORD GetTotalSendCount(){return m_dwTotalSendDataCount;};
		void SetSocketHandle(WORD Value){m_wSocket = Value;};
		WORD GetSocketHandle(){return m_wSocket;};
		void SetSocket(int socket){m_iSocket = socket;};
		int GetSocket(){return m_iSocket;};
		void SetIOCPHandle(HANDLE hIOCP){m_hIocp = hIOCP;};
		HANDLE GetIOCPHandle(){return m_hIocp;};
		pBlock GetBlock(bool bRead);

		
	};


}

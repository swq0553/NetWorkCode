#pragma once
#include "TypeDefine.h"
#include <string>
#include "CustomClientSocket.h"

using namespace std;

namespace TheOne
{

	class CCustomClientSocket;	
	class CCustomSocket
	{
	public:
		CCustomSocket(void);
		~CCustomSocket(void);

	protected:
		WSADATA     m_data;
        HANDLE      m_hIOCP;
		IOCPSOCKET m_IOCPSOCKET;
		

		virtual void SetActive(bool Value){}
		virtual bool ActiveStart(){return false;};
		virtual void ActiveStop(){};
		virtual void OnConnect(CCustomSocket * Socket){};
   	    virtual void OnDisConnect(CCustomSocket * Socket){};
 		virtual void OnRead(CCustomSocket * socket ,const char * pBuf,int nLen){};
		virtual bool onAccept(const char * pszIP ,int iport){return false;};
		virtual void OnError(CCustomSocket * socket ,int nErrorCode){};
		virtual void FreeClient(CCustomClientSocket * clientSocket){};

	public:
		bool Initialize(){return ActiveStart();};
		void Finalization(){ActiveStop();}
		void SocketErrorEvent(CCustomSocket * socket ,int ErrorCode){ OnError( socket,ErrorCode);};
		void Open(){ActiveStart();};
		void Close(){ActiveStop();};
		void SetServerIp(const string StrIP);
		string GetServerIp(){return m_IOCPSOCKET.ip;};
		int  GetServerPort(){return m_IOCPSOCKET.port;};
		void SetServerPort(int iport){if (m_IOCPSOCKET.socket == INVALID_SOCKET) m_IOCPSOCKET.port = iport;};
		bool GetActive(){return m_IOCPSOCKET.socket != INVALID_SOCKET;};
		void DoRead(CCustomSocket * socket , const char * pBuf ,int nlen){OnRead(socket , pBuf, nlen);};
		pIOCPSOCKET  GetIOCPSocket(){return &m_IOCPSOCKET;};
		HANDLE GetIOCPHandle(){return m_hIOCP;};
		void ReleaseClient(CCustomClientSocket * client){ FreeClient(client);};


	};


};


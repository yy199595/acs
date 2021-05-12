#pragma once
#include<Define/CommonDef.h>
#include<Module/Component.h>
namespace SoEasy
{
	enum SessionState
	{
		None,
		Connect, //正在连接
		Normal,	//正常
		ConnectError,	//错误
	};


	struct NetWorkPack
	{
	public:
		NetWorkPack(std::string & ip, unsigned int port, char * buf, size_t size)
			: mIp(ip), mPort(port), mMessage(buf, size) {}
	public:
		const std::string mIp;
		const unsigned short mPort;
		const std::string mMessage;
	};
	class SessionManager;
	
	class TcpClientSession : public std::enable_shared_from_this<TcpClientSession>
	{
		typedef std::function<void(shared_ptr<TcpClientSession>, bool)> ConnectCallback;
	public:
		TcpClientSession(SessionManager * manager, SharedTcpSocket socket);
		TcpClientSession(SessionManager * manager, std::string name, std::string ip, unsigned short port);
		virtual ~TcpClientSession();
	public:
		bool IsActive();	
		inline const std::string & GetIP() { return mIp; }
		inline unsigned short GetPort() { return mPort; }
		inline long long GetSocketId() { return mSocketId; }
		inline long long GetStartTime() { return mStartTime; }
		inline SessionState GetState() { return mCurrentSatte; }
		inline const bool IsContent() { return this->mIsContent; }
		inline const std::string & GetAddress() { return mAdress; }	
		inline SharedTcpSocket GetSocket() { return this->mBinTcpSocket; }
		inline const std::string & GetSessionName() { return mSessionName; }
		inline void SetSessionName(const std::string & name) { mSessionName = name; }		
	public:
		bool StartConnect(ConnectCallback action = nullptr);
		void InvokeConnectCallback();
	public:
		void CloseSocket();
		bool SendPackage(const std::string & message);
		bool SendPackage(const char * message, const size_t size);
		SessionManager * GetDispatchManager() { return this->mDispatchManager; }
	public:
		void StartReceiveMsg();
	private:
		
		void ReadMessageBody(const  size_t allSize);
		void InitMember(const std::string & ip, unsigned short port);
	private:
		std::string mIp;
		long long mSocketId;
		std::string mAdress;
		unsigned short mPort;
		AsioContext & mAsioContext;
		SharedTcpSocket mBinTcpSocket;
		AsioTcpEndPoint mSocketEndPoint;
		ConnectCallback mConnectCallback;
	private:
		bool mIsContent;
		long long mStartTime;	
		std::string mSessionName;
		SessionState mCurrentSatte;
		unsigned int mConnectCount;
		SessionManager * mDispatchManager;
	private:
		char * mRecvMsgBuffer;
		unsigned int mRecvBufferSize;
	};
	typedef shared_ptr<TcpClientSession> SharedTcpSession;
	
}
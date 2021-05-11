#pragma once
#include"Manager.h"
namespace SoEasy
{
	typedef std::function<void(shared_ptr<TcpClientSession>, shared_ptr<NetWorkPacket>)> RecvMsgCallback;
	class SessionManager : public Manager
	{
	public:
		SessionManager() : Manager(1) { }
		virtual ~SessionManager() { }
	public:
		friend TcpClientSession;
		shared_ptr<TcpClientSession> CreateTcpSession(SharedTcpSocket socket);
		shared_ptr<TcpClientSession> CreateTcpSession(std::string name, std::string address);
		shared_ptr<TcpClientSession> CreateTcpSession(std::string name, std::string ip, unsigned short port);
	public:
		void AddRecvCallback(RecvMsgCallback callback) { this->mRecvMsgCallback = callback; }
	private:	//不要手动调用
		bool AddNewSession(shared_ptr<TcpClientSession> tcpSession);
		bool AddErrorSession(shared_ptr<TcpClientSession> tcpSession);
		void AddRecvMessage(shared_ptr<TcpClientSession> tcpSession, const char * message, size_t size);
	protected:
		bool OnInit() override;
		virtual void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession) = 0;
		virtual void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession) = 0;
		virtual void OnRecvNewMessageAfter(const std::string & address, const char * msg, size_t size);
	private:
		XCode InvokeAction(shared_ptr<TcpClientSession> tcpSession, shared_ptr<NetWorkPacket> callInfo, shared_ptr<NetWorkPacket> returnData);
	protected:
		void OnSystemUpdate() final;
		long long GetIdByAddress(const std::string & address);
		shared_ptr<TcpClientSession> GetCurSession() { return this->mCurrentSession; }
		bool ParseAddress(const std::string & address, string & ip, unsigned short & port);
	protected:
		class NetWorkManager * mNetWorkManager;
	private:
		NetWorkPacket mNetWorkPacket;
		RecvMsgCallback mRecvMsgCallback;
		class LocalActionManager * mActionManager;
		class CoroutineManager * mCoroutineSheduler;
		shared_ptr<TcpClientSession> mCurrentSession;  //当前正在执行action的session
		DoubleBufferQueue<SharedNetPacket> mRecvMessageQueue;
		DoubleBufferQueue<shared_ptr<TcpClientSession>> mNewSessionQueue;
		DoubleBufferQueue<shared_ptr<TcpClientSession>> mErrorSessionQueue;
	};
}
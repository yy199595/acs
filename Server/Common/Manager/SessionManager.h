#pragma once
#include"Manager.h"
namespace SoEasy
{
	typedef std::function<bool(shared_ptr<TcpClientSession>, shared_ptr<NetWorkPacket>)> RecvMsgCallback;
	class SessionManager : public Manager
	{
	public:
		SessionManager() : Manager(1) { }
		virtual ~SessionManager() { }
	public:
		friend TcpClientSession;
		shared_ptr<TcpClientSession> CreateTcpSession(SharedTcpSocket socket);
		shared_ptr<TcpClientSession> CreateTcpSession(std::string name, std::string address);
		void SetRecvCallback(RecvMsgCallback callback) { this->mRecvMsgCallback = callback; }
		shared_ptr<TcpClientSession> CreateTcpSession(std::string name, std::string ip, unsigned short port);
	private:	//不要手动调用
		bool AddNewSession(SharedTcpSession tcpSession);
		bool AddErrorSession(SharedTcpSession tcpSession);
		void AddRecvMessage(SharedTcpSession tcpSession, const char * message, size_t size);
	protected:
		bool OnInit() override;
		virtual void OnSessionErrorAfter(SharedTcpSession tcpSession) = 0;
		virtual void OnSessionConnectAfter(SharedTcpSession tcpSession) = 0;
		virtual void OnRecvNewMessageAfter(SharedTcpSession tcpSession, shared_ptr<NetWorkPacket> packet);
	private:
		XCode InvokeAction(SharedTcpSession tcpSession, shared_ptr<NetWorkPacket> callInfo, shared_ptr<NetWorkPacket> returnData);
	protected:
		void OnSystemUpdate() final;
		virtual void OnSecondUpdate() override;
		long long GetIdByAddress(const std::string & address);
		SharedTcpSession GetCurSession() { return this->mCurrentSession; }
		bool ParseAddress(const std::string & address, string & ip, unsigned short & port);
	protected:
		class NetWorkManager * mNetWorkManager;
	private:
		int mReConnectTime;
		NetWorkPacket mNetWorkPacket;
		RecvMsgCallback mRecvMsgCallback;
		class LocalActionManager * mActionManager;
		class CoroutineManager * mCoroutineSheduler;
		shared_ptr<TcpClientSession> mCurrentSession;  //当前正在执行action的session
		DoubleBufferQueue<SharedNetPacket> mRecvMessageQueue;
		DoubleBufferQueue<SharedTcpSession> mNewSessionQueue;
		DoubleBufferQueue<SharedTcpSession> mErrorSessionQueue;
		std::unordered_map<std::string, SharedTcpSession> mWaitConnectSessionMap;	//要断线重连的session
	};
}
#pragma once
#include"Manager.h"
namespace SoEasy
{
	class CommandBase;
	class TelnetClientSession;

	struct CmdMessageBuffer
	{
	public:
		CmdMessageBuffer(const std::string & address, const std::string & data)
			: mAddress(address), mCmdData(data) { }
	public:
		const std::string mAddress;
		const std::string mCmdData;
	};

	typedef std::shared_ptr<CmdMessageBuffer> SharedCmdData;

	class CommandManager : public Manager
	{
	public:
		CommandManager();
		~CommandManager();
	public:
		friend class TelnetClientSession;
		bool AddCommandAction(const std::string gm, CommandBase * command);
		void AddCommandBackArgv(const std::string & address, XCode code, const std::string message);
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
		void OnFrameUpdate(float t) override;
	private:
		void Listen();
		void StartListen();
		bool InitListener();
	private:
		void AddErrorSession(std::shared_ptr<TelnetClientSession> session);
		void AddReceiveMessage(std::shared_ptr<TelnetClientSession> session, const std::string & message);
	private:	
		void OnSessionErrorAfter(std::shared_ptr<TelnetClientSession> session);
		void OnSessionConnectAfter(std::shared_ptr<TelnetClientSession> session);
		void OnRecvMessage(shared_ptr<TelnetClientSession> session, const std::string & message);
	private:
		std::set<std::string> mLoginUserList;
		std::set<std::string> mLoginAddressList;
	private:
		unsigned short mListenerPort;
		AsioTcpAcceptor * mBindAcceptor;
	private:
		DoubleBufferQueue<SharedCmdData> mRecvMessageQueue;
		DoubleBufferQueue<shared_ptr<TelnetClientSession>> mErrorSessionQueue;
	private:
		std::unordered_map<std::string, CommandBase *> mCommonActions;
		std::unordered_map<std::string, shared_ptr<TelnetClientSession>> mTcpSessionMap;

	};
}
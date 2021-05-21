#pragma once
#include"Manager.h"
#include<Other/Command.h>
namespace SoEasy
{
	class TelnetClientSession;
	typedef std::function<XCode(shared_ptr<TelnetClientSession>, const std::string &, RapidJsonWriter &)> CommandAction;
	class CommandManager : public Manager
	{
	public:
		CommandManager();
		~CommandManager();
	public:
		bool AddCommandAction(const std::string gm, CommandBase * command);
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
		void OnFrameUpdate(float t) override;

	private:
		void Listen();
		void StartListen();
		bool InitListener();
	protected:
		void OnSessionErrorAfter(std::shared_ptr<TelnetClientSession> session);
		void OnSessionConnectAfter(std::shared_ptr<TelnetClientSession> session);
	private:
		std::set<std::string> mLoginUserList;
		std::set<std::string> mLoginAddressList;
	private:
		unsigned short mListenerPort;
		AsioTcpAcceptor * mBindAcceptor;
	private:
		std::mutex mSessionLock;
		std::vector<std::string> mAllCommandList;
		std::queue<NetMessageBuffer *> mSendMessageQueue;
		std::unordered_map<std::string, CommandBase *> mCommonActions;
		std::unordered_map<std::string, shared_ptr<TelnetClientSession>> mTcpSessionMap;

	};
}
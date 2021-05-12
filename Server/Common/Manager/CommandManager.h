#pragma once
#include"SessionManager.h"
#include<Other/Command.h>
namespace SoEasy
{
	class TcpSessionListener;
	typedef std::function<XCode(shared_ptr<TcpClientSession>, const std::string &, RapidJsonWriter &)> CommandAction;
	class CommandManager : public SessionManager
	{
	public:
		CommandManager();
		~CommandManager();
	public:
		bool AddCommandAction(const std::string gm, CommandBase * command);
	private:
		XCode Login(shared_ptr<TcpClientSession> tcpSession, const std::string & commandData, RapidJsonWriter & returnData);
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
		void OnFrameUpdate(float t) override;
	private:
		shared_ptr<TcpClientSession> GetTcpSession(const std::string & address);
		XCode Invoke(shared_ptr<TcpClientSession> tcpSession, const std::string & name, const std::string & args, RapidJsonWriter & returnData);
	private:
		void HandleCommandMsgBack();
		bool CheckSessionIsLogin(shared_ptr<TcpClientSession>);
		bool SendMessageByAddress(const std::string & address, RapidJsonWriter & jsonData);
	protected:
		void OnSessionErrorAfter(SharedTcpSession) override;
		void OnSessionConnectAfter(SharedTcpSession) override;
		void OnRecvNewMessageAfter(SharedTcpSession, shared_ptr<NetWorkPacket>) override;
	private:
		std::set<std::string> mLoginUserList;
		std::set<std::string> mLoginAddressList;
	private:
		unsigned short mListenerPort;
	private:
		std::mutex mSessionLock;
		TcpSessionListener * mTcpListener;
		std::vector<std::string> mAllCommandList;
		std::queue<NetMessageBuffer *> mSendMessageQueue;
		std::unordered_map<std::string, CommandBase *> mCommonActions;
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mTcpSessionMap;

	};
}
#include"CommandManager.h"
#include"NetWorkManager.h"
#include<Core/Applocation.h>
#include<Core/TcpSessionListener.h>

namespace SoEasy
{

	CommandManager::CommandManager()
	{
		this->mTcpListener = nullptr;
		this->mTcpSessionMap.clear();
		this->mCommonActions.clear();
	}

	CommandManager::~CommandManager()
	{

	}

	bool CommandManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("CommondPort", this->mListenerPort));
		this->mTcpListener = make_shared<TcpSessionListener>(this, this->mListenerPort);
		SayNoAssertRetFalse_F(this->mTcpListener->InitListener());
		return true;
	}

	void CommandManager::OnInitComplete()
	{
		this->mTcpListener->StartAcceptConnect();
		this->AddCommandAction("stop", new StopCommand());
		this->AddCommandAction("state", new StateCommand());
		SayNoDebugInfo("start command listener port : " << this->mListenerPort);
	}

	bool CommandManager::AddCommandAction(const std::string gm, CommandBase * command)
	{
		auto iter = this->mCommonActions.find(gm);
		if (iter == this->mCommonActions.end())
		{
			this->mAllCommandList.push_back(gm);
			this->mCommonActions.insert(std::make_pair(gm, command));
			return true;
		}
		return false;
	}

	void CommandManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mTcpSessionMap.find(address);
		if (iter != this->mTcpSessionMap.end())
		{
			this->mTcpSessionMap.erase(iter);
		}
	}

	void CommandManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mTcpSessionMap.find(address);
		if (iter != this->mTcpSessionMap.end())
		{
			this->mTcpSessionMap.erase(iter);
		}
		mSessionLock.lock();
		tcpSession->StartReceiveMsg();
		mSessionLock.unlock();
		this->mTcpSessionMap.insert(std::make_pair(address, tcpSession));

		RapidJsonWriter jsonData;
		jsonData.AddParameter("code", (int)XCode::Successful);
		jsonData.AddParameter("Message", "Please enter your account and password (Login -account@psssword)");
		this->SendMessageByAddress(tcpSession->GetAddress(), jsonData);

	}

	void CommandManager::OnRecvNewMessageAfter(SharedTcpSession, shared_ptr<NetWorkPacket>)
	{
		
	}

	shared_ptr<TcpClientSession> CommandManager::GetTcpSession(const std::string & address)
	{
		auto iter = this->mTcpSessionMap.find(address);
		return iter != this->mTcpSessionMap.end() ? iter->second : nullptr;

	}

	XCode CommandManager::Invoke(shared_ptr<TcpClientSession> tcpSession, const std::string & name, const std::string & args, RapidJsonWriter & returnData)
	{		
		if (!this->CheckSessionIsLogin(tcpSession))
		{
			if (name == "login")
			{
				return this->Login(tcpSession, args, returnData);
			}
			returnData.AddParameter("Message", "Please enter your account and password (Login -account@psssword)");
			return XCode::Failure;
		}
		auto iter = this->mCommonActions.find(name);
		if (iter == this->mCommonActions.end())
		{
			returnData.AddParameter("Error", "GM " + name + " not exist");
			return XCode::CallFunctionNotExist;
		}
		CommandBase * pCommandBase = iter->second;
		return pCommandBase->Invoke(args, returnData);
	}

	void CommandManager::HandleCommandMsgBack()
	{
		while (!this->mSendMessageQueue.empty())
		{
			
		}
	}

	bool CommandManager::SendMessageByAddress(const std::string & address, RapidJsonWriter & jsonData)
	{
		
		return true;
	}

	void CommandManager::OnFrameUpdate(float t)
	{
		Manager::OnFrameUpdate(t);
		this->HandleCommandMsgBack();
	}

	bool CommandManager::CheckSessionIsLogin(shared_ptr<TcpClientSession> session)
	{
		const std::string & address = session->GetAddress();
		auto iter = this->mLoginAddressList.find(address);
		return iter != this->mLoginAddressList.end();
	}

	XCode CommandManager::Login(shared_ptr<TcpClientSession> tcpSession, const std::string & commandData, RapidJsonWriter & returnData)
	{
		size_t pos = commandData.find("@");
		if (pos == std::string::npos)
		{
			returnData.AddParameter("Error", "GM Parameter Error");
			return XCode::CommandArgsError;
		}
		std::string user = commandData.substr(0, pos);
		std::string passwd = commandData.substr(pos + 1);
		const std::string & address = tcpSession->GetAddress();
		if (this->mLoginAddressList.find(address) != this->mLoginAddressList.end())
		{
			returnData.AddParameter("Error", "You have logged in successfully");
			return XCode::Failure;
		}
		if (this->mLoginUserList.find(user) != this->mLoginUserList.end())
		{
			returnData.AddParameter("Error", "Your account has been logged in");
			return XCode::Failure;
		}
		this->mLoginUserList.insert(user);
		this->mLoginAddressList.insert(address);
		returnData.AddParameter("list", this->mAllCommandList);
		returnData.AddParameter("Message", "User Login Successful");
		SayNoDebugWarning("user = " << user << " passwd = " << passwd);
		return XCode::Successful;
	}
}

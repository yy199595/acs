#include"ConsoleManager.h"
#include<Other/Command.h>
#include<Core/Applocation.h>
#include<Util/StringHelper.h>

#include<NetWork/TelnetClientSession.h>
namespace SoEasy
{

	ConsoleManager::ConsoleManager()
	{
		this->mTcpSessionMap.clear();
		this->mCommonActions.clear();
	}

	ConsoleManager::~ConsoleManager()
	{

	}

	bool ConsoleManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("CommandPort", this->mListenerPort));
		SayNoAssertRetFalse_F(this->InitListener());
		return true;
	}

	void ConsoleManager::OnInitComplete()
	{
		this->StartListen();
		this->AddCommandAction("state", new StateCommand(this));
		this->AddCommandAction("list", new ServiceListCommand(this));
		this->AddCommandAction("call", new ServiceCallCommand(this));
		SayNoDebugInfo("start command listener port : " << this->mListenerPort);
	}

	bool ConsoleManager::AddCommandAction(const std::string gm, CommandBase * command)
	{
		auto iter = this->mCommonActions.find(gm);
		if (iter == this->mCommonActions.end())
		{
			this->mCommonActions.insert(std::make_pair(gm, command));
			return true;
		}
		return false;
	}

	void ConsoleManager::AddCommandBackArgv(const std::string & address, XCode code, const std::string message)
	{
		auto iter = this->mTcpSessionMap.find(address);
		if (iter != this->mTcpSessionMap.end())
		{
			std::stringstream returnCommand;
			returnCommand << "\t<code = " << code << ">\n";
			returnCommand << "\t" << message << "\n";
			iter->second->StartWriteMessage(returnCommand.str());
		}
	}

	void ConsoleManager::OnSessionErrorAfter(std::shared_ptr<TelnetClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mTcpSessionMap.find(address);
		if (iter != this->mTcpSessionMap.end())
		{
			this->mTcpSessionMap.erase(iter);
			SayNoDebugError("remove telnet client : " << tcpSession->GetAddress());
		}
	}

	void ConsoleManager::OnSessionConnectAfter(std::shared_ptr<TelnetClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mTcpSessionMap.find(address);
		if (iter != this->mTcpSessionMap.end())
		{
			this->mTcpSessionMap.erase(iter);
		}
		
		const std::string welcome = "welcome to soeasy server console";

		tcpSession->StartRecvMessage();
		tcpSession->StartWriteMessage(welcome);
		this->mTcpSessionMap.emplace(address, tcpSession);
	}

	void ConsoleManager::OnRecvMessage(std::shared_ptr<TelnetClientSession> session, const std::string & message)
	{
		std::vector<std::string> commandArgvs;
		StringHelper::SplitString(message, "\r\n", commandArgvs);
		const std::string & command = commandArgvs[0];
		auto iter = this->mCommonActions.find(command);
		if (iter != this->mCommonActions.end())
		{
			if (commandArgvs.size() == 1)
			{
				iter->second->Invoke(session);
				return;
			}
			iter->second->Invoke(session, commandArgvs[1]);
		}
	}

	void ConsoleManager::Listen()
	{
		AsioContext & ioContext = this->GetApp()->GetAsioContext();
		SharedTcpSocket tcpSocket = std::make_shared<AsioTcpSocket>(ioContext);
		this->mBindAcceptor->async_accept(*tcpSocket, [this, &ioContext, tcpSocket](const asio::error_code & code)
		{
			if (!code)
			{
				unsigned short port = tcpSocket->remote_endpoint().port();
				std::string ip = tcpSocket->remote_endpoint().address().to_string();
				this->OnSessionConnectAfter(make_shared<TelnetClientSession>(this, tcpSocket));
			}
			ioContext.post(std::bind(&ConsoleManager::Listen, this));
		});
	}

	void ConsoleManager::StartListen()
	{
		AsioContext & ioContext = this->GetApp()->GetAsioContext();
		ioContext.post(std::bind(&ConsoleManager::Listen, this));
	}

	bool ConsoleManager::InitListener()
	{
		if (this->mBindAcceptor == nullptr)
		{
			try
			{
				AsioContext & io = this->GetApp()->GetAsioContext();
				AsioTcpEndPoint bindPoint(asio::ip::tcp::v4(), this->mListenerPort);
				this->mBindAcceptor = new AsioTcpAcceptor(io, bindPoint);
			}
			catch (const asio::system_error& e)
			{
				SayNoDebugError("start server fail " << e.what());
				return false;
			}
			return true;
		}
		return false;
	}

	void ConsoleManager::AddErrorSession(std::shared_ptr<TelnetClientSession> session)
	{
		this->mErrorSessionQueue.AddItem(session);
	}

	void ConsoleManager::AddReceiveMessage(std::shared_ptr<TelnetClientSession> session, const std::string & message)
	{
		const std::string & address = session->GetAddress();
		this->mRecvMessageQueue.AddItem(make_shared<CmdMessageBuffer>(address, message));
	}




	void ConsoleManager::OnFrameUpdate(float t)
	{
		SharedTelnetSession telnetSession;
		this->mRecvMessageQueue.SwapQueueData();
		this->mErrorSessionQueue.SwapQueueData();
		while (this->mErrorSessionQueue.PopItem(telnetSession))
		{
			this->OnSessionErrorAfter(telnetSession);
		}
		SharedCmdData commandData;
		while (this->mRecvMessageQueue.PopItem(commandData))
		{
			const std::string & address = commandData->mAddress;
			auto iter = this->mTcpSessionMap.find(address);
			if (iter != this->mTcpSessionMap.end())
			{
				telnetSession = iter->second;
				this->OnRecvMessage(telnetSession, commandData->mCmdData);
			}
		}
	}

}

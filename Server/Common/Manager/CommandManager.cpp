#include"CommandManager.h"
#include<Core/Applocation.h>
#include<NetWork/TelnetClientSession.h>
namespace SoEasy
{

	CommandManager::CommandManager()
	{
		this->mTcpSessionMap.clear();
		this->mCommonActions.clear();
	}

	CommandManager::~CommandManager()
	{

	}

	bool CommandManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("CommandPort", this->mListenerPort));
		SayNoAssertRetFalse_F(this->InitListener());
		return true;
	}

	void CommandManager::OnInitComplete()
	{
		this->StartListen();
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

	void CommandManager::OnSessionErrorAfter(std::shared_ptr<TelnetClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mTcpSessionMap.find(address);
		if (iter != this->mTcpSessionMap.end())
		{
			this->mTcpSessionMap.erase(iter);
		}
	}

	void CommandManager::OnSessionConnectAfter(std::shared_ptr<TelnetClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mTcpSessionMap.find(address);
		if (iter != this->mTcpSessionMap.end())
		{
			this->mTcpSessionMap.erase(iter);
		}
		this->mTcpSessionMap.emplace(address, tcpSession);
		const std::string welcome = "welcome server console";
		tcpSession->StartRecvMessage();
		tcpSession->StartWriteMessage(welcome);
	}

	void CommandManager::Listen()
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
			ioContext.post(std::bind(&CommandManager::Listen, this));
		});
	}

	void CommandManager::StartListen()
	{
		AsioContext & ioContext = this->GetApp()->GetAsioContext();
		ioContext.post(std::bind(&CommandManager::Listen, this));
	}

	bool CommandManager::InitListener()
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




	void CommandManager::OnFrameUpdate(float t)
	{
		Manager::OnFrameUpdate(t);

	}

}

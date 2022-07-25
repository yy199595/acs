
#include"TelnetClientContext.h"
#include"Util/StringHelper.h"
#include <Define/CommonLogDef.h>
#include"Component/Telnet/ConsoleComponent.h"
namespace Tcp
{
	TelnetClientContext::TelnetClientContext(std::shared_ptr<SocketProxy> socketProxy, ConsoleComponent * component)
		: Tcp::TcpContext(socketProxy), mConsoleComponent(component)
	{
		this->mSocket = socketProxy;
	}

	void TelnetClientContext::StartRead()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLine();
#else
		IAsioThread & netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&TelnetClientContext::ReceiveLine, this);
#endif
	}

	void TelnetClientContext::OnReceiveLine(const asio::error_code& code, std::istream & is)
	{
		if(code)
		{
			this->CloseContext();
			CONSOLE_LOG_ERROR(code.message());
			return;
		}
		std::string lineMessage;
		std::getline(is, lineMessage);
		const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mConsoleComponent->OnReceive(address, lineMessage);
#else
		IAsioThread & netWorkThread = App::Get()->GetTaskScheduler();
		netWorkThread.Invoke(&ConsoleComponent::OnReceive, this->mConsoleComponent,address , lineMessage);
#endif
	}

	void TelnetClientContext::SendProtoMessage(std::shared_ptr<TelnetProto> message)
	{
#ifdef ONLY_MAIN_THREAD
		this->Send(message);
#else
		IAsioThread &netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&TelnetClientContext::Send, this, message);
#endif
	}

	void TelnetClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code)
		{
			this->CloseContext();
			CONSOLE_LOG_ERROR(code.message());
		}
	}

	void TelnetClientContext::CloseContext()
	{
		this->mSocket->Close();
		const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mConsoleComponent->OnClientError(address);
#else
		IAsioThread & netWorkThread = App::Get()->GetTaskScheduler();
		netWorkThread.Invoke(&ConsoleComponent::OnClientError, this->mConsoleComponent, address);
#endif
	}
}


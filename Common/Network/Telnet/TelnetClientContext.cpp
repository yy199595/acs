
#include"TelnetClientContext.h"
#include"Util/StringHelper.h"
#include <Define/CommonLogDef.h>
#include"DB/Mongo/MongoProto.h"
#include"DB/Mongo/Bson/bson/bsonelement.h"
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
		netWorkThread.Invoke(&TelnetClientContext::ReceiveMessage, this, sizeof(Mongo::MongoHead));
#endif
	}

	void TelnetClientContext::OnReceiveMessage(const asio::error_code& code, asio::streambuf& buffer)
	{
		std::iostream oss(&buffer);
		size_t size = buffer.size();
		if(this->mRequest == nullptr)
		{
			this->mRequest = std::make_shared<Mongo::MongoQueryRequest>();
			oss.readsome((char*)&this->mRequest->header, sizeof(Mongo::MongoHead));
			this->ReceiveMessage(this->mRequest->header.messageLength - sizeof(Mongo::MongoHead));
		}
		else
		{
			size_t offset = 0;
			char * buff = new char[buffer.size()];
			size_t length = oss.readsome(buff, buffer.size());
			std::memcpy(&this->mRequest->flag, buff, sizeof(int));
			offset += sizeof(int);


			this->mRequest->collectionName.append(buff + offset);
			offset += (this->mRequest->collectionName.size() + 1);

			std::memcpy(&this->mRequest->numberToSkip, buff + offset, sizeof(int));

			offset+= sizeof(int);

			std::memcpy(&this->mRequest->numberToReturn, buff + offset, sizeof(int));

			offset += sizeof(int);

			size_t total = 0;
			std::memcpy(&total, buff + offset, sizeof(int));


			std::string str(buff + offset, length - offset);
			minibson::document document(buff + offset, length - offset);

			oss.readsome((char *)&this->mRequest->numberToSkip, sizeof(this-mRequest->numberToSkip));
			oss.readsome((char *)&this->mRequest->numberToReturn, sizeof(this-mRequest->numberToReturn));

		}
	}

	void TelnetClientContext::OnReceiveLine(const asio::error_code& code, asio::streambuf& buffer)
	{
		if(code)
		{
			this->CloseContext();
			CONSOLE_LOG_ERROR(code.message());
			return;
		}
		std::string lineMessage;
		std::iostream os(&buffer);
		std::getline(os, lineMessage);
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


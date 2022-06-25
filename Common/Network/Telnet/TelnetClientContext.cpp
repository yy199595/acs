
#include"TelnetClientContext.h"
#include"Util/StringHelper.h"
#include <Define/CommonLogDef.h>
#include"DB/Mongo/MongoProto.h"
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
			oss.readsome((char *)&this->mRequest->flag, sizeof(int));

			char cc = oss.get();
			while(cc != '\0')
			{
				this->mRequest->collectionName += cc;
				cc = oss.get();
			}
			int length = 0;
			oss.readsome((char *)&this->mRequest->numberToSkip, sizeof(int));
			oss.readsome((char *)&this->mRequest->numberToReturn, sizeof(int));
			oss.readsome((char *)&length, sizeof(int));

			std::string json;
			Bson::BsonDocumentNode bsonDocumentNode(oss, length);
			bsonDocumentNode.WriterToJson(json);

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


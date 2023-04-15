#include"TcpListenerComponent.h"
#include"Network/Tcp/SocketProxy.h"
#include"Rpc/Method/MethodProxy.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Component/ThreadComponent.h"
#include"Entity/Unit/App.h"
#include"Network/Tcp/Asio.h"
namespace Tendo
{
	TcpListenerComponent::TcpListenerComponent()
    {
        this->mListenCount = 0;
        this->mBindAcceptor = nullptr;
        this->mThreadComponent = nullptr;
    }

	TcpListenerComponent::~TcpListenerComponent()
	{
		Asio::Code err;
		this->mBindAcceptor->close(err);
	}

    bool TcpListenerComponent::StopListen()
    {
		if(this->mBindAcceptor != nullptr)
		{
			asio::error_code code;
			this->mBindAcceptor->close(code);
			if (code)
			{
				CONSOLE_LOG_ERROR(code.message());
				return false;
			}
		}
        return true;
    }

	bool TcpListenerComponent::StartListen(const char * name)
    {
        unsigned short port = 0;
        this->mThreadComponent = this->GetComponent<ThreadComponent>();
        if(!ServerConfig::Inst()->GetListen(name, this->mNet, port))
        {
            LOG_ERROR("not find listen config " << name);
            return false;
        }
        try
        {
            Asio::Context& io = this->mApp->MainThread();
            Asio::EndPoint ep(asio::ip::address_v4(), port);
			this->mBindAcceptor = std::make_unique<Asio::Acceptor>(io);

			this->mListenPort = port;
            this->mBindAcceptor->open(ep.protocol());
            this->mBindAcceptor->bind(ep);
            this->mBindAcceptor->listen();
            io.post(std::bind(&TcpListenerComponent::ListenConnect, this));
            LOG_INFO(this->GetName() << " listen [" << port << "] successful");
            return true;
        }
        catch (std::system_error & err)
        {
            LOG_ERROR(fmt::format("{0}  listen [{1}] failure {2}", this->GetName(), port, err.what()));
            return false;
        }
    }
	void TcpListenerComponent::ListenConnect()
	{
        std::shared_ptr<Tcp::SocketProxy> socketProxy
            = this->mThreadComponent->CreateSocket(this->mNet);
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(),
			[this, socketProxy](const asio::error_code & code)
		{
            if(code == asio::error::operation_aborted) //强制取消
            {
                this->mBindAcceptor = nullptr;
                CONSOLE_LOG_ERROR("close listen " << this->GetName() << " [" << this->mListenPort << "]");
                this->OnStopListen();
                return;
            }
			if (code)
			{
                socketProxy->Close();
				CONSOLE_LOG_FATAL(this->GetName() << " " << code.message());
			}
			else
			{
                socketProxy->Init();
                this->mListenCount++;
                this->OnListen(socketProxy);
                CONSOLE_LOG_DEBUG(socketProxy->GetAddress() << " connect to " << this->GetName());
            }
            Asio::Context& io = this->mApp->MainThread();
            io.post(std::bind(&TcpListenerComponent::ListenConnect, this));
		});
	}
}

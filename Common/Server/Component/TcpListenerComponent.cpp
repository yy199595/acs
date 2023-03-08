#include"TcpListenerComponent.h"
#include"Tcp/SocketProxy.h"
#include"Method/MethodProxy.h"
#include"Log/CommonLogDef.h"
#include"Config/ServerConfig.h"
#include"Component/ThreadComponent.h"
namespace Sentry
{
	TcpListenerComponent::TcpListenerComponent()
    {
		this->mCount = 0;
        this->mErrorCount = 0;
        this->mBindAcceptor = nullptr;
        this->mNetComponent = nullptr;
    }

	TcpListenerComponent::~TcpListenerComponent()
	{
		Asio::Code err;
		this->mBindAcceptor->close(err);
		delete this->mBindAcceptor;
	}

    bool TcpListenerComponent::StopListen()
    {
        asio::error_code code;
        this->mBindAcceptor->close(code);
        if(code)
        {
            CONSOLE_LOG_ERROR(code.message());
            return false;
        }
        return true;
    }

	bool TcpListenerComponent::StartListen(const char * name)
    {
        unsigned short port = 0;
        this->mNetComponent = this->GetComponent<ThreadComponent>();
        if(!ServerConfig::Inst()->GetListen(name, port))
        {
            LOG_ERROR("not find listen config " << name);
            return false;
        }
        try
        {
            Asio::Context& io = this->mNetComponent->GetContext();
            this->mBindAcceptor = new Asio::Acceptor (io,
				Asio::EndPoint(asio::ip::address_v4(), port));

            this->mIsClose = false;
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
        std::shared_ptr<SocketProxy> socketProxy 
            = this->mNetComponent->CreateSocket();
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(),
			[this, socketProxy](const asio::error_code & code)
		{
            if(code == asio::error::operation_aborted) //强制取消
            {
                delete this->mBindAcceptor;
                this->mBindAcceptor = nullptr;
                CONSOLE_LOG_ERROR("close listen " << this->GetName());
                this->OnStopListen();
                return;
            }
			if (code)
			{
				this->mErrorCount++;
                socketProxy->Close();
				CONSOLE_LOG_FATAL(this->GetName() << " " << code.message() << " count = " << this->mErrorCount);
			}
			else
			{
				this->mCount++;
                socketProxy->Init();
                if(!this->OnListen(socketProxy))
                {
                    CONSOLE_LOG_ERROR(this->GetName() << " stop listen ...");
                    return ;
                }
                CONSOLE_LOG_DEBUG(socketProxy->GetAddress() << " connect to " << this->GetName());
            }
            Asio::Context& io = this->mApp->MainThread();
            io.post(std::bind(&TcpListenerComponent::ListenConnect, this));
		});
	}
}

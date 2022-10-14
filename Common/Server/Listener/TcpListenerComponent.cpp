#include"TcpListenerComponent.h"
#include"App/App.h"
#include"Tcp/SocketProxy.h"
#include"Method/MethodProxy.h"
#include"Log/CommonLogDef.h"
#include"Component/NetThreadComponent.h"
namespace Sentry
{
	TcpListenerComponent::TcpListenerComponent()
    {
		this->mCount = 0;
        this->mConfig = nullptr;
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
        const ServerConfig * config = ServerConfig::Get();
        LOG_CHECK_RET_FALSE(this->mConfig = config->GetListenConfig(name));
        try
        {
            if(this->mConfig == nullptr)
            {
                return false;
            }
            unsigned short port = this->mConfig->Port;
            Asio::Context & io = this->GetApp()->GetThread();
            this->mBindAcceptor = new Asio::Acceptor (io, 
                Asio::EndPoint(asio::ip::make_address(this->mConfig->Ip), port));
            this->mNetComponent = this->GetComponent<NetThreadComponent>();

            this->mIsClose = false;
            this->mBindAcceptor->listen();
            io.post(std::bind(&TcpListenerComponent::ListenConnect, this));
            LOG_INFO(this->mConfig->Name << " listen [" << this->mConfig->Address << "] successful");           
            return true;
        }
        catch (std::system_error & err)
        {
            LOG_ERROR(fmt::format("listen {0}:{1} failure {2}",
                                  this->mConfig->Ip, this->mConfig->Port, err.what()));
            return false;
        }
    }
	void TcpListenerComponent::ListenConnect()
	{
        std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(),
			[this, socketProxy](const asio::error_code & code)
		{
            if(code == asio::error::operation_aborted) //强制取消
            {
                delete this->mBindAcceptor;
                this->mBindAcceptor = nullptr;
                CONSOLE_LOG_ERROR("close listen " << this->mConfig->Name);
                this->OnStopListen();
                return;
            }
			if (code)
			{
				this->mErrorCount++;
                socketProxy->Close();
				CONSOLE_LOG_FATAL(this->mConfig->Name << " " << code.message() << " count = " << this->mErrorCount);
			}
			else
			{
				this->mCount++;
                socketProxy->Init();
                if(!this->OnListen(socketProxy))
                {
                    CONSOLE_LOG_ERROR("stop listen " << this->mConfig->Address);
                    return ;
                }
                CONSOLE_LOG_DEBUG(socketProxy->GetAddress() << " connect to " << this->mConfig->Name);
            }
            Asio::Context& io = this->GetApp()->GetThread();
            io.post(std::bind(&TcpListenerComponent::ListenConnect, this));
		});
	}
}

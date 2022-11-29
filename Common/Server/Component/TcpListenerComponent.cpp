#include"TcpListenerComponent.h"
#include"Tcp/SocketProxy.h"
#include"Method/MethodProxy.h"
#include"Log/CommonLogDef.h"
#include"Config/ServerConfig.h"
#include"Component/NetThreadComponent.h"
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
        std::string address;
        if(!ServerConfig::Inst()->GetListen(name, address))
        {
            LOG_ERROR("not find listen config " << name);
            return false;
        }
        size_t pos = address.find(":");
        if(pos == std::string::npos)
        {
            return false;
        }
        const std::string ip = address.substr(0, pos);
        const std::string port = address.substr(pos + 1);
        try
        {
            Asio::Context & io = this->mApp->MainThread();
            this->mBindAcceptor = new Asio::Acceptor (io, 
                Asio::EndPoint(asio::ip::make_address(ip), std::stoi(port)));
            this->mNetComponent = this->GetComponent<NetThreadComponent>();

            this->mIsClose = false;
            this->mAddress = address;
            this->mBindAcceptor->listen();
            io.post(std::bind(&TcpListenerComponent::ListenConnect, this));
            LOG_INFO(this->GetName() << " listen [" << this->mAddress << "] successful");
            return true;
        }
        catch (std::system_error & err)
        {
            LOG_ERROR(fmt::format("listen {0} failure {2}", this->mAddress, err.what()));
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
                    CONSOLE_LOG_ERROR("stop listen " << this->mAddress);
                    return ;
                }
                CONSOLE_LOG_DEBUG(socketProxy->GetAddress() << " connect to " << this->GetName());
            }
            Asio::Context& io = this->mApp->MainThread();
            io.post(std::bind(&TcpListenerComponent::ListenConnect, this));
		});
	}
}

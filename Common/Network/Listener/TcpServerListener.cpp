#include"TcpServerListener.h"
#include"App/App.h"
#include<Network/SocketProxy.h>
#include<Method/MethodProxy.h>
#include<Define/CommonLogDef.h>
#include"TcpServerComponent.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	TcpServerListener::TcpServerListener(const ListenConfig *config)
    {
		this->mCount = 0;
        this->mConfig = config;
        this->mErrorCount = 0;
        this->mTcpComponent = nullptr;
        this->mBindAcceptor = nullptr;
        this->mNetComponent = nullptr;
    }

	TcpServerListener::~TcpServerListener()
	{
		asio::error_code err;
		this->mBindAcceptor->close(err);
		delete this->mBindAcceptor;
	}

	bool TcpServerListener::StartListen(asio::io_service &io, TcpServerComponent *component)
    {
        try
        {
            this->mTcpComponent = component;
            unsigned short port = this->mConfig->Port;
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), port);
            this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);
            this->mNetComponent = App::Get()->GetComponent<NetThreadComponent>();

            this->mBindAcceptor->listen();
            this->ListenConnect();
            return true;
        }
        catch (std::system_error & err)
        {
            LOG_FATAL(fmt::format("listen {0}:{1} failure {2}",
                                  this->mConfig->Ip, this->mConfig->Port, err.what()));
            return false;
        }
    }
	void TcpServerListener::ListenConnect()
	{
        std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(),
			[this, socketProxy](const asio::error_code & code)
		{
			if (code)
			{
				this->mErrorCount++;
                this->mNetComponent->DeleteSocket(socketProxy);
				CONSOLE_LOG_FATAL(this->mConfig->Name << " " << code.message() << " count = " << this->mErrorCount);
			}
			else
			{
				this->mCount++;
                socketProxy->Init();
                this->mTcpComponent->OnListenConnect(this->mConfig->Name, socketProxy);
            }
            this->ListenConnect();
		});
	}
}

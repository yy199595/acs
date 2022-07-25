#include"TcpServerListener.h"
#include"App/App.h"
#include<Network/SocketProxy.h>
#include<Method/MethodProxy.h>
#include<Define/CommonLogDef.h>
#include<Thread/TaskThread.h>
#include"TcpServerComponent.h"
#include"Component/Scene/NetThreadComponent.h"

namespace Sentry
{
	TcpServerListener::TcpServerListener(IAsioThread & t)
		: mTaskThread(t)
    {
		this->mCount = 0;
		this->mErrorCount = 0;
        this->mConfig = nullptr;
        this->mBindAcceptor = nullptr;
    }

	TcpServerListener::~TcpServerListener()
	{
		asio::error_code err;
		this->mBindAcceptor->close(err);
		delete this->mBindAcceptor;
	}

	bool TcpServerListener::StartListen(const ListenConfig *config, TcpServerComponent *component)
    {
        try
        {
            this->mConfig = config;
            this->mTcpComponent = component;
            unsigned short port = config->Port;
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), port);
            this->mBindAcceptor = new AsioTcpAcceptor(this->mTaskThread, endPoint);

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
        IAsioThread & workThread = this->mTcpComponent->GetThread();
        std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread));
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(),
			[this, &workThread, socketProxy](const asio::error_code & code)
		{
			if (code)
			{
				this->mErrorCount++;
				CONSOLE_LOG_FATAL(this->mConfig->Name << " " << code.message() << " count = " << this->mErrorCount);
			}
			else
			{
				this->mCount++;
                socketProxy->Init();
                this->mTcpComponent->OnListenConnect(this->mConfig->Name, socketProxy);
            }
			this->mTaskThread.post(std::bind(&TcpServerListener::ListenConnect, this));
		});
	}
}

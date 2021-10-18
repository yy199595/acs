#include "NetworkListener.h"
#include<Core/App.h>
#include<Method/MethodProxy.h>
#include<Define/CommonDef.h>
#include<Thread/TaskThread.h>
#include<Component/IComponent.h>
namespace Sentry
{
	NetworkListener::NetworkListener(NetWorkThread * t, ListenConfig & config)
		: mTaskThread(t), mConfig(config),
        mTaskScheduler(App::Get().GetTaskScheduler())
    {
        this->mBindAcceptor = nullptr;
        this->mSessionHandler = nullptr;
    }

	void NetworkListener::StartListen(ISocketHandler * handler)
	{
		this->mSessionHandler = handler;
        this->mTaskThread->AddTask(NewMethodProxy(&NetworkListener::ListenConnect, this));
	}

	void NetworkListener::ListenConnect()
	{
        if(this->mBindAcceptor == nullptr)
        {
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mConfig.Port);
            this->mBindAcceptor = new AsioTcpAcceptor(this->mTaskThread->GetContext(), endPoint);
            try
            {
                asio::error_code err;
                this->mBindAcceptor->listen(this->mConfig.Count, err);

                SayNoDebugInfo("start " << this->mConfig.Name << " listener "
                                        << this->mBindAcceptor->local_endpoint().address().to_string() << ":" <<
                                        this->mBindAcceptor->local_endpoint().port() << "  max count = " << this->mConfig.Count
                                        << "}");
            }
            catch(asio::system_error & err)
            {
                SayNoDebugError(err.what());
            }
        }
		this->mSessionSocket = this->mSessionHandler->CreateSocket();
		this->mBindAcceptor->async_accept(this->mSessionSocket->GetSocket(), std::bind(&NetworkListener::OnConnectHandler, this, args1));
	}

	void NetworkListener::OnConnectHandler(const asio::error_code & code)
	{
		if (!code)
		{			
#ifdef _DEBUG
			const std::string & address = this->mSessionSocket->GetAddress();
			SayNoDebugError(this->mConfig.Name << " listen new socket " << address);
#endif
            this->mSessionSocket->OnListenDone();
			this->mTaskScheduler.AddMainTask(NewMethodProxy(
				&ISocketHandler::OnListenConnect, this->mSessionHandler, this->mSessionSocket));
            SayNoDebugInfo(this->mConfig.Name << " listen new socket " << this->mSessionSocket->GetAddress());
		}
		else
		{		
			mSessionSocket->Close();
			delete this->mSessionSocket;
			SayNoDebugError(code.message());
		}
		this->mTaskThread->GetContext().post(std::bind(&NetworkListener::ListenConnect, this));
	}
}

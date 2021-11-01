#include"NetworkListener.h"
#include<Core/App.h>
#include<Method/MethodProxy.h>
#include<Define/CommonDef.h>
#include<Thread/TaskThread.h>
#include<Component/IComponent.h>
namespace GameKeeper
{
	NetworkListener::NetworkListener(NetWorkThread * t, ListenConfig & config)
		: mTaskThread(t), mConfig(config),
        mTaskScheduler(App::Get().GetTaskScheduler())
    {
        this->mIsListen = false;
        this->mBindAcceptor = nullptr;
        this->mSessionHandler = nullptr;
    }

	NetworkListener::~NetworkListener()
	{
		asio::error_code err;
		this->mBindAcceptor->close(err);
		delete this->mBindAcceptor;
	}

	bool NetworkListener::StartListen(ISocketHandler * handler)
	{
		this->mSessionHandler = handler;
        this->mTaskThread->AddTask(&NetworkListener::InitListener, this);
        App::Get().GetCorComponent()->YieldReturn(this->mCorId);
		return this->mIsListen;
	}

    void NetworkListener::InitListener()
    {
        try
        {
            asio::io_service & io = this->mTaskThread->GetContext();
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mConfig.Port);
            this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);

            this->mBindAcceptor->listen(this->mConfig.Count);
            io.post(std::bind(&NetworkListener::ListenConnect, this));
            this->mIsListen = true;
        }
        catch (std::system_error & err)
        {
            this->mIsListen = false;
            SayNoDebugFatal("listen " << this->mConfig.Ip << ":"
                                      << this->mConfig.Port << " failure" << err.what());
        }
        CoroutineComponent * component = App::Get().GetCorComponent();
        this->mTaskScheduler.AddMainTask(&CoroutineComponent::Resume, component, this->mCorId);
    }

	void NetworkListener::ListenConnect()
	{
		this->mSessionSocket = this->mSessionHandler->CreateSocket();
		this->mBindAcceptor->async_accept(this->mSessionSocket->GetSocket(), std::bind(&NetworkListener::OnConnectHandler, this, args1));
	}

	void NetworkListener::OnConnectHandler(const asio::error_code & code)
	{
        this->mSessionSocket->OnListenDone(code);
        if(!code)
        {
            SayNoDebugInfo(this->mConfig.Name << " listen new socket " << this->mSessionSocket->GetAddress());
        }
        this->mTaskThread->GetContext().post(std::bind(&NetworkListener::ListenConnect, this));
	}
}

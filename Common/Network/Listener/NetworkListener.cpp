#include"NetworkListener.h"
#include<Core/App.h>
#include<Network/SocketProxy.h>
#include<Method/MethodProxy.h>
#include<Define/CommonDef.h>
#include<Thread/TaskThread.h>
#include<Component/IComponent.h>
#include<Scene/TaskPoolComponent.h>
namespace GameKeeper
{
	NetworkListener::NetworkListener(NetWorkThread & t, ListenConfig & config)
		: mTaskThread(t), mConfig(config),
        mTaskScheduler(App::Get().GetTaskScheduler())
    {
        this->mIsListen = false;
        this->mBindAcceptor = nullptr;
        this->mListenHandler = nullptr;
		mTaskComponent = App::Get().GetComponent<TaskPoolComponent>();
    }

	NetworkListener::~NetworkListener()
	{
		asio::error_code err;
		this->mBindAcceptor->close(err);
		delete this->mBindAcceptor;
	}

	bool NetworkListener::StartListen(ISocketListen * handler)
	{
		this->mListenHandler = handler;
        this->mTaskThread.Invoke(&NetworkListener::InitListener, this);
        App::Get().GetCorComponent()->YieldReturn(this->mCorId);
		return this->mIsListen;
	}

    void NetworkListener::InitListener()
    {
        try
        {
            asio::io_service & io = this->mTaskThread.GetContext();
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mConfig.Port);
            this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);

            this->mBindAcceptor->listen(this->mConfig.Count);
            io.post(std::bind(&NetworkListener::ListenConnect, this));
            this->mIsListen = true;
        }
        catch (std::system_error & err)
        {
            this->mIsListen = false;
            GKDebugFatal("listen " << this->mConfig.Ip << ":"
                                      << this->mConfig.Port << " failure" << err.what());
        }
        CoroutineComponent * component = App::Get().GetCorComponent();
        this->mTaskScheduler.Invoke(&CoroutineComponent::Resume, component, this->mCorId);
    }

	void NetworkListener::ListenConnect()
	{		
		auto socketProxy = new SocketProxy(this->mTaskComponent->AllocateNetThread(), this->mConfig.Name);
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(), 
			[this, socketProxy](const asio::error_code & code)
		{
			if (!code)
			{
#ifdef __DEBUG__
				AsioTcpSocket & socket = socketProxy->GetSocket();
				unsigned short port = socket.remote_endpoint().port();
				const std::string ip = socket.remote_endpoint().address().to_string();
				GKDebugInfo(this->mConfig.Name << " listen new socket " << ip << ":" << port);
#endif // __DEBUG__
                mTaskScheduler.Invoke(&ISocketListen::OnListen, this->mListenHandler, socketProxy);
			}
			else
			{
				delete socketProxy;
			}
			AsioContext & context = this->mTaskThread.GetContext();
			context.post(std::bind(&NetworkListener::ListenConnect, this));
		});
	}
}

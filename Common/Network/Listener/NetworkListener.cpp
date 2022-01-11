#include"NetworkListener.h"
#include<Core/App.h>
#include<Network/SocketProxy.h>
#include<Method/MethodProxy.h>
#include<Define/CommonLogDef.h>
#include<Thread/TaskThread.h>
#include<Component/IComponent.h>
#include<Scene/ThreadPoolComponent.h>
namespace GameKeeper
{
	NetworkListener::NetworkListener(NetWorkThread & t, ListenConfig & config)
		: mTaskThread(t), mConfig(config),
        mTaskScheduler(App::Get().GetTaskScheduler())
    {
        this->mIsListen = false;
        this->mBindAcceptor = nullptr;
        this->mListenHandler = nullptr;
		mTaskComponent = App::Get().GetComponent<ThreadPoolComponent>();
    }

	NetworkListener::~NetworkListener()
	{
		asio::error_code err;
		this->mBindAcceptor->close(err);
		delete this->mBindAcceptor;
	}

	std::shared_ptr<TaskSource<bool>> NetworkListener::StartListen(ISocketListen * handler)
	{
		this->mListenHandler = handler;
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
        this->mTaskThread.Invoke(&NetworkListener::InitListener, this, taskSource);
		return taskSource;
	}

    void NetworkListener::InitListener(std::shared_ptr<TaskSource<bool>> taskSource)
    {
        try
        {
            asio::io_service & io = this->mTaskThread.GetContext();
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mConfig.Port);
            this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);

            this->mBindAcceptor->listen(this->mConfig.Count);
            io.post(std::bind(&NetworkListener::ListenConnect, this));
            taskSource->SetResult(true);
        }
        catch (std::system_error & err)
        {
            taskSource->SetResult(false);
            LOG_FATAL("listen {0}:{1} failure {2}",
                      this->mConfig.Ip, this->mConfig.Port, err.what());
        }
    }

	void NetworkListener::ListenConnect()
	{
        NetWorkThread & workThread = this->mTaskComponent->AllocateNetThread();
        std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread, this->mConfig.Name));
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(),
			[this, socketProxy](const asio::error_code & code)
		{
			if (!code)
			{
#ifdef __DEBUG__
				AsioTcpSocket & socket = socketProxy->GetSocket();
				unsigned short port = socket.remote_endpoint().port();
				const std::string ip = socket.remote_endpoint().address().to_string();
                LOG_INFO("{0}:{1} connected {2}", ip, port, this->mConfig.Name);
#endif // __DEBUG__
                mTaskScheduler.Invoke(&ISocketListen::OnListen, this->mListenHandler, socketProxy);
			}
			AsioContext & context = this->mTaskThread.GetContext();
			context.post(std::bind(&NetworkListener::ListenConnect, this));
		});
	}
}

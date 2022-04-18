#include"NetworkListener.h"
#include"App/App.h"
#include<Network/SocketProxy.h>
#include<Method/MethodProxy.h>
#include<Define/CommonLogDef.h>
#include<Thread/TaskThread.h>
#include<Component/IComponent.h>
#include"Component/Scene/ThreadPoolComponent.h"
namespace Sentry
{
	NetworkListener::NetworkListener(IAsioThread & t, ListenConfig & config)
		: mTaskThread(t), mConfig(config),
        mTaskScheduler(App::Get()->GetTaskScheduler())
    {
        this->mIsListen = false;
        this->mBindAcceptor = nullptr;
        this->mListenHandler = nullptr;
		mTaskComponent = App::Get()->GetComponent<ThreadPoolComponent>();
    }

	NetworkListener::~NetworkListener()
	{
		asio::error_code err;
		this->mBindAcceptor->close(err);
		delete this->mBindAcceptor;
	}

	bool NetworkListener::StartListen(ISocketListen * handler)
    {
		if(this->mIsListen)
		{
			return true;
		}
        this->mListenHandler = handler;
#ifdef ONLY_MAIN_THREAD
        try
        {
            asio::io_service & io = this->mTaskThread.GetContext();
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mConfig.Port);
            this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);

            this->mBindAcceptor->listen(this->mConfig.Count);
            std::string str = this->mBindAcceptor->local_endpoint().address().to_string();
            io.post(std::bind(&NetworkListener::ListenConnect, this));
			this->mIsListen = true;
            return true;
        }
        catch (std::system_error & err)
        {
            LOG_FATAL(fmt::format("listen {0}:{1} failure {2}",
                                  this->mConfig.Ip, this->mConfig.Port, err.what()));
            return false;
        }
#else
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
        this->mTaskThread.Invoke(&NetworkListener::InitListener, this, taskSource);
        taskSource->Await();
#endif
    }
#ifndef ONLY_MAIN_THREAD
    void NetworkListener::InitListener(std::shared_ptr<TaskSource<bool>> taskSource)
    {
        try
        {
            asio::io_service & io = this->mTaskThread.GetContext();
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mConfig.Port);
            this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);

            this->mBindAcceptor->listen(this->mConfig.Count);
            std::string str = this->mBindAcceptor->local_endpoint().address().to_string();
            io.post(std::bind(&NetworkListener::ListenConnect, this));
            taskSource->SetResult(true);
			this->mIsListen = true;
        }
        catch (std::system_error & err)
        {
            taskSource->SetResult(false);
            LOG_FATAL(fmt::format("listen {0}:{1} failure {2}",
                      this->mConfig.Ip, this->mConfig.Port, err.what()));
        }
    }
#endif
	void NetworkListener::ListenConnect()
	{
#ifdef ONLY_MAIN_THREAD
        MainTaskScheduler & workThread = App::Get()->GetTaskScheduler();
#else
        IAsioThread & workThread = this->mTaskComponent->AllocateNetThread();
#endif
        std::shared_ptr<AsioTcpSocket> tcpSocket(new AsioTcpSocket(workThread.GetContext()));
		this->mBindAcceptor->async_accept(*tcpSocket,
			[this, &workThread, tcpSocket](const asio::error_code & code)
		{
			if (!code)
            {
                std::shared_ptr<SocketProxy> socketProxy(
                        new SocketProxy(workThread, tcpSocket));
#ifdef __DEBUG__
				LOG_INFO(socketProxy->GetAddress() << " connect to " << this->mConfig.Name);
#endif
#ifdef ONLY_MAIN_THREAD
                this->mListenHandler->OnListen(socketProxy);
#else
                this->mTaskScheduler.Invoke(&ISocketListen::OnListen, this->mListenHandler, socketProxy);
#endif
            }
			AsioContext & context = this->mTaskThread.GetContext();
			context.post(std::bind(&NetworkListener::ListenConnect, this));
		});
	}
}

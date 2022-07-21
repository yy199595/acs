#include"NetworkListener.h"
#include"App/App.h"
#include<Network/SocketProxy.h>
#include<Method/MethodProxy.h>
#include<Define/CommonLogDef.h>
#include<Thread/TaskThread.h>
#include<Component/IComponent.h>
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	NetworkListener::NetworkListener(IAsioThread & t, const ListenConfig & config)
		: mTaskThread(t), mConfig(config)
    {
        this->mBindAcceptor = nullptr;
        this->mListenHandler = nullptr;
		mTaskComponent = App::Get()->GetComponent<NetThreadComponent>();
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
#ifdef ONLY_MAIN_THREAD
        try
        {
            //asio::io_service & io = this->mTaskThread.GetContext();
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mConfig.Port);
            this->mBindAcceptor = new AsioTcpAcceptor(this->mTaskThread, endPoint);

            this->mBindAcceptor->listen(this->mConfig.Count);
            this->ListenConnect();
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
        return taskSource->Await();
#endif
    }
#ifndef ONLY_MAIN_THREAD
    void NetworkListener::InitListener(std::shared_ptr<TaskSource<bool>> taskSource)
    {
        try
        {
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mConfig.Port);
            this->mBindAcceptor = new AsioTcpAcceptor(this->mTaskThread, endPoint);

            this->mBindAcceptor->listen(this->mConfig.Count);
            this->mTaskThread.Invoke(&NetworkListener::ListenConnect, this);
            taskSource->SetResult(true);
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
        std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread));
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(),
			[this, &workThread, socketProxy](const asio::error_code & code)
		{
			if (!code)
            {
                socketProxy->Init();
#ifdef __DEBUG__
				LOG_INFO(socketProxy->GetAddress() << " connect to " << this->mConfig.Name);
#endif
#ifdef ONLY_MAIN_THREAD
                this->mListenHandler->OnListen(socketProxy);
#else
                IAsioThread & t = App::Get()->GetTaskScheduler();
                t.Invoke(&ISocketListen::OnListen, this->mListenHandler, socketProxy);
#endif
            }
			this->mTaskThread.post(std::bind(&NetworkListener::ListenConnect, this));
		});
	}
}

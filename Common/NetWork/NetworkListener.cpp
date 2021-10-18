#include "NetworkListener.h"
#include<Core/App.h>
#include<Method/MethodProxy.h>
#include<Define/CommonDef.h>
#include<Thread/TaskThread.h>
#include<Component/IComponent.h>
namespace Sentry
{
	NetworkListener::NetworkListener(const std::string & name, NetWorkThread * t, unsigned short port, int maxCount)
		: mName(name), mTaskThread(t), mPort(port), mMaxCount(maxCount), mTaskScheduler(App::Get().GetTaskScheduler())
	{
		this->mBindAcceptor = nullptr;
		this->mSessionHandler = nullptr;
		AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mPort);
		this->mBindAcceptor = new AsioTcpAcceptor(t->GetContext(), endPoint);
	}

	void NetworkListener::StartListen(ISocketHandler * handler)
	{
		asio::error_code err;
		this->mSessionHandler = handler;
		this->mBindAcceptor->listen(this->mMaxCount, err);	
		if (!err)
		{
			SayNoDebugInfo("start " << this->mName << "listener "
				<< this->mBindAcceptor->local_endpoint().address().to_string() << ":" <<
				this->mBindAcceptor->local_endpoint().port() << "  max count = " << this->mMaxCount << "}");

			this->mSessionHandler->SetNetThread(mTaskThread);
			this->mTaskThread->AddTask(new FucntionTask<NetworkListener>(&NetworkListener::ListenConnect, this));			
		}
		else
		{
			SayNoDebugError(err.message());
			App::Get().Stop();
		}
	}

	void NetworkListener::ListenConnect()
	{
		this->mSessionSocket = this->mSessionHandler->CreateSocket(this->mTaskThread->GetContext());
		this->mBindAcceptor->async_accept(this->mSessionSocket->GetSocket(), std::bind(&NetworkListener::OnConnectHandler, this, args1));
	}

	void NetworkListener::OnConnectHandler(const asio::error_code & code)
	{
		if (!code)
		{			
#ifdef _DEBUG
			const std::string & address = this->mSessionSocket->GetAddress();
			SayNoDebugError(this->mName << " listen new socket " << address);
#endif
			this->mSessionSocket->StartReceive();
			this->mTaskScheduler.AddMainTask(NewMethodProxy(
				&ISocketHandler::OnListenConnect, this->mSessionHandler, this->mSessionSocket));
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

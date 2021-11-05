//
// Created by zmhy0073 on 2021/10/27.
//

#include "TcpLocalSession.h"
#include <Core/App.h>
#include<Network/Tcp/TcpClientComponent.h>
namespace GameKeeper
{
	TcpLocalSession::TcpLocalSession(TcpClientComponent * component, const std::string & ip, const unsigned short port)
		:TcpClientSession(component), mIp(ip), mPort(port)
	{
		this->mAddress = ip + ":" + std::to_string(mPort);
	}

    void TcpLocalSession::StartConnect()
    {		
		GKAssertRet_F(this->mSocketProxy);
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		nThread.AddTask(&TcpLocalSession::ConnectHandler, this);
    }

    void TcpLocalSession::StartAsyncConnect()
    {
		GKAssertRet_F(this->mSocketProxy);
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
        unsigned int id = App::Get().GetCorComponent()->GetCurrentCorId();
		nThread.AddTask(&TcpLocalSession::AsyncConnectHandler, this, id);
        App::Get().GetCorComponent()->YieldReturn();
    }

	void TcpLocalSession::ConnectHandler()
    {
        this->mConnectCount++;
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
        GKDebugLog(this->mSocketProxy->GetName() << " start connect " << this->GetAddress());
		nSocket.async_connect(endPoint, [this](const asio::error_code &err)
        {
            if(!err)
            {
				this->StartReceive();
                this->mConnectCount = 0;			
            }
			NetWorkThread & nThread = this->mSocketProxy->GetThread();
			nThread.AddTask(&TcpClientComponent::OnConnectRemoteAfter, this->mTcpComponent, this, err);
        });
    }

    void TcpLocalSession::AsyncConnectHandler(unsigned int id)
    {
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		GKDebugLog(this->mSocketProxy->GetName() << " start connect " << this->GetAddress());
		nSocket.async_connect(endPoint, [this, id](const asio::error_code &err)
        {
            if (!err)
            {
				this->StartReceive();
				this->mConnectCount = 0;
            }		
			NetWorkThread & nThread = this->mSocketProxy->GetThread();
			CoroutineComponent *component = App::Get().GetCorComponent();
			MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();       
			taskScheduler.AddMainTask(&CoroutineComponent::Resume, component, id);	
			nThread.AddTask(&TcpClientComponent::OnConnectRemoteAfter, this->mTcpComponent, this, err);
        });
    }
}
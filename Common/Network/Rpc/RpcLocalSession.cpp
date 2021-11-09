//
// Created by zmhy0073 on 2021/10/27.
//

#include "RpcLocalSession.h"
#include <Core/App.h>
#include<Network/Rpc/RpcComponent.h>
namespace GameKeeper
{
	RpcLocalSession::RpcLocalSession(RpcComponent * component, const std::string & ip, const unsigned short port)
		:RpcClientSession(component), mIp(ip), mPort(port)
	{
		this->mAddress = ip + ":" + std::to_string(mPort);
	}

    void RpcLocalSession::StartConnect()
    {		
		GKAssertRet_F(this->mSocketProxy);
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		nThread.AddTask(&RpcLocalSession::ConnectHandler, this);
    }

    void RpcLocalSession::StartAsyncConnect()
    {
		GKAssertRet_F(this->mSocketProxy);
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
        unsigned int id = App::Get().GetCorComponent()->GetCurrentCorId();
		nThread.AddTask(&RpcLocalSession::AsyncConnectHandler, this, id);
        App::Get().GetCorComponent()->YieldReturn();
    }

	void RpcLocalSession::ConnectHandler()
    {
        this->mConnectCount++;
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
        GKDebugLog(this->mSocketProxy->GetName() << " start connect " << this->GetAddress());
		nSocket.async_connect(endPoint, [this](const asio::error_code &err)
        {
			XCode code = XCode::Successful;
            if(!err)
            {
				this->StartReceive();
                this->mConnectCount = 0;
				code = XCode::NetConnectFailure;
            }
			NetWorkThread & nThread = this->mSocketProxy->GetThread();
			nThread.AddTask(&RpcComponent::OnConnectRemoteAfter, this->mTcpComponent, this, code);
        });
    }

    void RpcLocalSession::AsyncConnectHandler(unsigned int id)
    {
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		GKDebugLog(this->mSocketProxy->GetName() << " start connect " << this->GetAddress());
		nSocket.async_connect(endPoint, [this, id](const asio::error_code &err)
        {
			//XCode code = XCode::Successful;
            if (!err)
            {
				this->StartReceive();
				this->mConnectCount = 0;
				//code = XCode::NetConnectFailure;
            }		
			NetWorkThread & nThread = this->mSocketProxy->GetThread();
			CoroutineComponent *component = App::Get().GetCorComponent();
			MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();       
			taskScheduler.AddMainTask(&CoroutineComponent::Resume, component, id);	
			//nThread.AddTask(&RpcComponent::OnConnectRemoteAfter, this->mTcpComponent, this, code);
        });
    }
}
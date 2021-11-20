//
// Created by zmhy0073 on 2021/10/27.
//

#include "ProtoRpcConnector.h"
#include <Core/App.h>
#include<Component/Scene/ProtoRpcComponent.h>
namespace GameKeeper
{
	ProtoRpcConnector::ProtoRpcConnector(ProtoRpcComponent * component, SocketProxy * socket)
		:ProtoRpcClient(component, socket)
	{	
        this->mIsConnect = false;
	}

	void ProtoRpcConnector::StartConnect(std::string & ip, unsigned short port, StaticMethod * method)
	{
		this->mIsConnect = true;
		GKAssertRet_F(this->mSocketProxy);
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		nThread.AddTask(&ProtoRpcConnector::ConnectHandler, this, ip, port, method);
	}

	void ProtoRpcConnector::ConnectHandler(std::string & ip, unsigned short port,  StaticMethod * method)
    {
        this->mConnectCount++;
        auto address = asio::ip::make_address_v4(ip);
        asio::ip::tcp::endpoint endPoint(address, port);
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
        GKDebugLog(this->mSocketProxy->GetName() << " start connect " << this->GetAddress());
		nSocket.async_connect(endPoint, [this, method](const asio::error_code &err)
        {
			XCode code = XCode::Successful;
            if(!err)
            {
				this->StartReceive();
                this->mConnectCount = 0;
				code = XCode::NetConnectFailure;
            }
            this->mIsConnect = false;
            MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
            if(method != nullptr)
            {
                taskScheduler.AddMainTask(method);
                return;
            }
            taskScheduler.AddMainTask(&ProtoRpcComponent::OnConnectRemoteAfter, this->mTcpComponent, this, code);
        });
    }
}
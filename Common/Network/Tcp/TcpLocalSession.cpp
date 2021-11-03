//
// Created by zmhy0073 on 2021/10/27.
//

#include "TcpLocalSession.h"
#include <Core/App.h>
namespace GameKeeper
{
    TcpLocalSession::TcpLocalSession(ISocketHandler *handler, const std::string &name, const std::string ip,
                                     const unsigned short port)
                                     : TcpClientSession(handler, name)
    {
        this->mIp = ip;
        this->mPort = port;
        this->mAddress = ip + ":" + std::to_string(port);
    }

    void TcpLocalSession::ConnecRemote()
    {
        if(this->IsActive())
        {
            return;
        }
        this->mHandler.GetNetThread()->AddTask(&TcpLocalSession::ConnectHandler, this);
    }

    void TcpLocalSession::AsyncConnectRemote()
    {
        if(this->IsActive())
        {
            return;
        }
        unsigned int id = App::Get().GetCorComponent()->GetCurrentCorId();
        this->mHandler.GetNetThread()->AddTask(&TcpLocalSession::AsyncConnectHandler, this, id);
        App::Get().GetCorComponent()->YieldReturn();
    }

    void TcpLocalSession::ConnectHandler()
    {
        this->mConnectCount++;
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
        GKDebugLog(this->GetName() << " start connect " << this->GetAddress());
        this->mSocket->async_connect(endPoint, [this](const asio::error_code &err)
        {
            if(!err)
            {
                this->mIsOpen = true;
                this->mConnectCount = 0;
                this->OnSessionEnable();
            }
            this->OnConnect(err);
        });
    }

    void TcpLocalSession::AsyncConnectHandler(unsigned int id)
    {
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
        GKDebugLog(this->GetName() << " start connect " << this->GetAddress());
        this->mSocket->async_connect(endPoint, [this, id](const asio::error_code &err)
        {
            if (!err)
            {
                this->mIsOpen = true;
                this->mConnectCount = 0;
                this->OnSessionEnable();
            }
            this->OnConnect(err);
            CoroutineComponent *component = App::Get().GetCorComponent();
            this->mTaskScheduler.AddMainTask(&CoroutineComponent::Resume, component, id);
        });
    }
}
//
// Created by zmhy0073 on 2021/10/27.
//

#include "TcpLocalSession.h"
#include <Core/App.h>
namespace Sentry
{
    void TcpLocalSession::ConnectByAddress(const std::string &name, const std::string &ip, unsigned short port)
    {
        if(this->IsActive())
        {
            return;
        }
        this->mIp = ip;
        this->mName = name;
        this->mPort = port;
        this->mHandler->GetNetThread()->AddTask(&TcpLocalSession::ConnectHandler, this);
    }

    void TcpLocalSession::AsyncConnectByAddress(const std::string &name, const std::string &ip, unsigned short port)
    {
        if(this->IsActive())
        {
            return;
        }
        this->mIp = ip;
        this->mName = name;
        this->mPort = port;
        unsigned int id = App::Get().GetCorComponent()->GetCurrentCorId();
        this->mHandler->GetNetThread()->AddTask(&TcpLocalSession::AsyncConnectHandler, this, id);
        App::Get().GetCorComponent()->YieldReturn();
    }

    void TcpLocalSession::ConnectHandler()
    {
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
        SayNoDebugLog(this->mName << " start connect " << this->mIp << this->mPort);
        this->mSocket->async_connect(endPoint, [this](const asio::error_code &err)
        {
            if(!err)
            {
                this->InitMember();
            }
            this->OnConnect(err);
        });
    }

    void TcpLocalSession::AsyncConnectHandler(unsigned int id)
    {
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
        SayNoDebugLog(this->mName << " start connect " << this->mIp << this->mPort);
        this->mSocket->async_connect(endPoint, [this, id](const asio::error_code &err)
        {
            if(!err)
            {
                this->InitMember();
            }
            CoroutineComponent * component = App::Get().GetCorComponent();
            this->mTaskScheduler.AddMainTask(&CoroutineComponent::Resume, component, id);
        });
    }
}
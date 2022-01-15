//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpClient.h"
#include<iostream>
#include<stdlib.h>
#include"Util/StringHelper.h"
#include"Define/CommonLogDef.h"
namespace GameKeeper
{
    TcpClient::TcpClient(std::shared_ptr<SocketProxy> socket, SocketType type)
        : mContext(socket->GetContext()), mNetworkThread(socket->GetThread())
    {
        this->mSocket = socket;
        this->mSocketType = type;
    }

    bool TcpClient::StartConnect(const std::string &address)
    {
        if(this->mIsConnected)
        {
            return false;
        }
        std::string ip;
        unsigned short port = 0;
        this->mIsConnected = true;
        if(!Helper::String::ParseIpAddress(address, ip, port))
        {
            STD_ERROR_LOG("parse address error : " << address);
            return false;
        }
        this->mNetworkThread.Invoke(&TcpClient::ConnectByAddress, this, ip, port);
        return true;
    }

    void TcpClient::StartReceive()
    {
        this->mIsOpen = true;
        this->mNetworkThread.Invoke(&TcpClient::BeginReceive, this);
    }

    void TcpClient::ConnectByAddress(const std::string &ip, unsigned short port)
    {
        AsioTcpSocket  & tcpSocket = this->mSocket->GetSocket();
        auto address = asio::ip::make_address_v4(ip);
        asio::ip::tcp::endpoint endPoint(address, port);
        tcpSocket.async_connect(endPoint, [this]
            (const asio::error_code & code)
        {
            if(code)
            {
                this->mIsOpen = false;
                STD_ERROR_LOG(code.message());
            }
            this->mIsConnected = false;
            this->OnConnect(code);
        });
    }

    bool TcpClient::SendDataByArray(const char *buffer, size_t size, int tag)
    {
        if(!this->mIsOpen)
        {
            return false;
        }
        AsioTcpSocket  & tcpSocket = this->mSocket->GetSocket();
        tcpSocket.async_send(asio::buffer(buffer, size), [this, tag](
                const asio::error_code & error, const size_t size)
        {
            if(error)
            {
                this->mIsOpen = false;
                STD_ERROR_LOG(error.message());
            }
            this->OnSendData(error, tag);
        });
        return true;
    }
}
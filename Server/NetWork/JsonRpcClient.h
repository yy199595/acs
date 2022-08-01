//
// Created by yjz on 22-7-31.
//

#ifndef SERVER_JSONRPCCLIENT_H
#define SERVER_JSONRPCCLIENT_H
#include<istream>
#include"Json/JsonReader.h"
#include"Network/TcpContext.h"
#include"google/protobuf/message.h"
using namespace google::protobuf;

namespace Sentry
{
    class JsonClientComponent;
}

namespace Tcp
{
    class JsonMessage : public ProtoMessage
    {
    public:
        JsonMessage(const std::string & json);
    public:
        int Serailize(std::ostream & os) final; //返回剩余要发送的字节数
    private:
        Json::Reader mJson;
        std::string mMessage;
    };

    class JsonRequest
    {
    public:
        JsonRequest();
    public:
        bool ParseMessage(const std::string & address, std::istream & is);

    public:
        long long GetRpcId() const { return this->mRpcId;}
        const std::string & GetFunc() const { return this->mFunc;}
        const std::string & GetData() const { return this->mData;}
        const std::string & GetAddress() const { return this->mAddress;}
    private:
        long long mRpcId;
        std::string mData;
        std::string mFunc;
        Json::Reader mJson;
        std::string mAddress;
    };
}

namespace Tcp
{

    class JsonRpcClient : public TcpContext
    {
    public:
        JsonRpcClient(std::shared_ptr<SocketProxy> socket, JsonClientComponent * component);
    public:
        void StartReceive();
        void SendMesageData(const std::string & json);
    private:
        void OnReceiveLine(const asio::error_code &code, std::istream &readStream) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream &readStream) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;

    private:
        JsonClientComponent * mGateComponent;
    };
}


#endif //SERVER_JSONRPCCLIENT_H

//
// Created by yjz on 22-7-31.
//

#include"JsonRpcClient.h"
#include"App/App.h"
#include"google/protobuf/util/json_util.h"
#include"Component/Gate/JsonClientComponent.h"
namespace Tcp
{
    JsonMessage::JsonMessage(const std::string & json)
    {
        this->mMessage = json;
    }

    int JsonMessage::Serailize(std::ostream &os)
    {
        std::string line = std::to_string(this->mMessage.size() + 2);
        os << line << "\r\n";
        os.write(this->mMessage.c_str(), this->mMessage.size());
        return 0;
    }

    JsonRequest::JsonRequest()
    {
        this->mRpcId = 0;
    }

    bool JsonRequest::ParseMessage(const std::string &address, std::istream &is)
    {
        size_t size = is.rdbuf()->in_avail();
        std::unique_ptr<char[]> buffer(new char[size]);
        if (!this->mJson.ParseJson(buffer.get(), size))
        {
            return false;
        }
        this->mAddress = address;
        this->mJson.GetMember("func", this->mFunc);
        this->mJson.GetMember("rpc_id", this->mRpcId);
        this->mJson.GetMember("data", this->mData);
        return true;
    }
}

namespace Tcp
{
    JsonRpcClient::JsonRpcClient(std::shared_ptr<SocketProxy> socket, JsonClientComponent * component)
        : TcpContext(socket)
    {
        
    }

    void JsonRpcClient::SendMesageData(const std::string & json)
    {
        std::shared_ptr<JsonMessage> data(new JsonMessage(json));
#ifdef ONLY_MAIN_THREAD
        this->Send(data);
#else
        asio::io_service & io = this->mSocket->GetThread();
        io.post(std::bind(&JsonRpcClient::Send, this, data));
#endif
    }

    void JsonRpcClient::StartReceive()
    {
#ifdef ONLY_MAIN_THREAD
        this->ReceiveLine();
#else
        asio::io_service & io = this->mSocket->GetThread();
        io.post(std::bind(&JsonRpcClient::ReceiveLine, this));
#endif
    }

    void JsonRpcClient::OnReceiveLine(const asio::error_code &code, std::istream &readStream)
    {
        std::string line;
        std::getline(readStream, line);
        assert(line.back() == '\r');
        line.pop_back();
        this->ReceiveMessage(std::stoi(line));
    }

    void JsonRpcClient::OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message)
    {
        if(code)
        {
            CONSOLE_LOG_ERROR(code.message());
            return;
        }
        this->PopMessage();
        this->SendFromMessageQueue();
    }

    void JsonRpcClient::OnReceiveMessage(const asio::error_code &code, std::istream &readStream)
    {
        const std::string & address = this->mSocket->GetAddress();
        std::shared_ptr<JsonRequest> jsonRequest(new JsonRequest());
        if(!jsonRequest->ParseMessage(address, readStream))
        {
            CONSOLE_LOG_ERROR("parse json request message error");
        }
        else
        {
#ifdef ONLY_MAIN_THREAD
            this->mGateComponent->OnRequest(jsonRequest);
#else
            asio::io_service & io = this->mSocket->GetThread();
            io.post(std::bind(&JsonClientComponent::OnRequest, this->mGateComponent, jsonRequest));
#endif
        }
        this->ReceiveLine();
    }
}
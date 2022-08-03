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
        size_t len = is.readsome(buffer.get(), size);
        CONSOLE_LOG_INFO("json = " << std::string(buffer.get(), len));
        if (!this->mJson.ParseJson(buffer.get(), size))
        {
            return false;
        }
        this->mAddress = address;
        std::vector<const rapidjson::Value *> params;
        assert(this->mJson.GetMember("params", params));
        assert(this->mJson.GetMember("id", this->mRpcId));
        assert(this->mJson.GetMember("method", this->mFunc));
        assert(!params.empty());
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
        io.post(std::bind(&JsonRpcClient::ReceiveSomeMessage, this));
#endif
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
            asio::io_service & io = App::Get()->GetThread();
            io.post(std::bind(&JsonClientComponent::OnRequest, this->mGateComponent, jsonRequest));
#endif
        }
        this->ReceiveLine();
    }
}
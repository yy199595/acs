#include"ClientManager.h"
#include<Util/MathHelper.h>
#include<Util/StringHelper.h>

#include<Protocol/db.pb.h>
#include<Pool/ObjectPool.h>

#include<Manager/ActionManager.h>

#include<Other/ObjectFactory.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>

namespace Client
{
    bool ClientManager::OnInit()
    {
        this->GetConfig().GetValue("ListenAddress", "ip", this->mConnectIp);
        this->GetConfig().GetValue("ListenAddress", "port", this->mConnectPort);
        this->mCoroutineManager = this->GetManager<CoroutineManager>();
        return NetProxyManager::OnInit();;
    }

    void ClientManager::OnInitComplete()
    {
        this->ConnectByAddress(this->mAddress, "GateServer");
        this->mCoroutineManager->Start(BIND_THIS_ACTION_0(ClientManager::InvokeAction));
    }

    void ClientManager::OnFrameUpdate(float t)
    {
        if (!this->mWaitSendMessages.empty())
        {
            TcpProxySession *tcpSession = this->GetProxySession(this->mAddress);
            if (tcpSession != nullptr)
            {
                NetMessageProxy *messageData = this->mWaitSendMessages.front();
                tcpSession->SendMessageData(messageData);
                this->mWaitSendMessages.pop();
            }
        }
    }

    XCode ClientManager::Notice(const std::string &service, const std::string &method)
    {
        return XCode();
    }

    XCode ClientManager::Notice(const std::string &service, const std::string &method, const Message &request)
    {
        return XCode();
    }

    XCode ClientManager::Invoke(const std::string &service, const std::string &method)
    {
        return XCode();
    }

    XCode ClientManager::Invoke(const std::string &service, const std::string &method, const Message &request)
    {
        return XCode();
    }

    XCode ClientManager::Call(const std::string &service, const std::string &method, Message &response)
    {


        return XCode();
    }

    XCode ClientManager::Call(const std::string &service, const std::string &method, const Message &request,
                              Message &response)
    {
        return XCode::Successful;
    }

    void ClientManager::InvokeAction()
    {

        s2s::MysqlQuery_Request requestData;
        s2s::MysqlQuery_Response responseData;

        db::UserAccountData userAccountData;

        userAccountData.set_userid(13716061995);

        requestData.set_protocolname(userAccountData.GetTypeName());
        requestData.set_protocolmessage(userAccountData.SerializeAsString());
    }
}


#include"ClientComponent.h"
#include"App/App.h"
#include"String/StringHelper.h"
#include"Client/TcpRpcClientContext.h"
#include"Lua/Client.h"
#include"Lua/Message.h"
#include"google/protobuf/util/json_util.h"
#include"Component/LuaScriptComponent.h"
#include"Component/NetThreadComponent.h"
namespace Client
{
	ClientTask::ClientTask(int ms)
        : Sentry::IRpcTask<Rpc::Data>(ms)
	{
		this->mTaskId = Guid::Create();
	}

	void ClientTask::OnResponse(std::shared_ptr<Rpc::Data> response)
	{
		this->mTask.SetResult(response);
	}

    void ClientTask::OnTimeout()
    {
        std::shared_ptr<Rpc::Data> response(new Rpc::Data());

        response->GetHead().Add("code", (int)XCode::CallTimeout);
        this->mTask.SetResult(response);
    }

}

namespace Client
{
	ClientComponent::ClientComponent()
	{
		this->mPort = 0;
		this->mLuaComponent = nullptr;
        this->mTimerComponent = nullptr;
	}

    void ClientComponent::OnRequest(std::shared_ptr<c2s::rpc::call> t1)
    {
        LOG_INFO("call client func = " << t1->func());
	}

    void ClientComponent::OnMessage(const std::string &address, std::shared_ptr<Rpc::Data> message)
    {
        Tcp::Type type = (Tcp::Type)message->GetType();
        Tcp::Porto proto = (Tcp::Porto)message->GetProto();
        switch(type)
        {
            case Tcp::Type::Request:
            {

            }
                break;
            case Tcp::Type::Response:
                long long rpcId = 0;
                if(message->GetHead().Get("rpc", rpcId))
                {
                    this->OnResponse(rpcId, message);
                }
                break;
        }
    }

    void ClientComponent::StartClose(const std::string &address)
    {

    }

    void ClientComponent::OnCloseSocket(const std::string &address, XCode code)
    {

    }

    bool ClientComponent::LateAwake()
    {
        this->mTimerComponent = this->GetComponent<TimerComponent>();
		this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
		return true;
    }

	std::shared_ptr<Rpc::Data> ClientComponent::Call(std::shared_ptr<Rpc::Data> request)
    {
        std::shared_ptr<ClientTask> clienRpcTask(new ClientTask(0));

        request->GetHead().Add("rpc", clienRpcTask->GetRpcId());

        this->mTcpClient->SendToServer(request);
        return this->AddTask(clienRpcTask)->Await();
    }

	void ClientComponent::OnTimeout(long long rpcId)
	{
		this->OnResponse(rpcId, nullptr);
	}

	void ClientComponent::OnAddTask(RpcTask rpctask)
	{
		//LOG_WARN(this->GetName() << " add new task " << rpctask->GetRpcId());
	}

	bool ClientComponent::StartConnect(const std::string& ip, unsigned short port)
	{
        NetThreadComponent * netComponent = this->GetComponent<NetThreadComponent>();
		std::shared_ptr<SocketProxy> socketProxy = netComponent->CreateSocket(ip, port);
		this->mTcpClient = std::make_shared<TcpRpcClientContext>(socketProxy, this);

		return true;
	}

	void ClientComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<ClientComponent>();
		luaRegister.PushExtensionFunction("Call", Lua::ClientEx::Call);
		luaRegister.PushExtensionFunction("StartConnectAsync", Lua::ClientEx::StartConnect);
	}
}

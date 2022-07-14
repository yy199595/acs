
#include"ClientComponent.h"
#include"App/App.h"
#include<Util/StringHelper.h>
#include"Network/TcpRpcClientContext.h"
#include"Task/ClientRpcTask.h"
#include"Script/Client.h"
#include"Script/Extension/Message/Message.h"
#include"Component/Http/HttpComponent.h"
#include"google/protobuf/util/json_util.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Component/Scene/NetThreadComponent.h"

namespace Client
{
	ClientComponent::ClientComponent()
	{
        this->mTimerComponent = nullptr;
	}

	unsigned int ClientComponent::AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms)
	{
		long long rpcId = task->GetRpcTaskId();
		//this->mRpcTasks.emplace(rpcId, task);
		return ms > 0 ? this->mTimerComponent->DelayCall(ms, &ClientComponent::OnTimeout, this, rpcId) : 0;
	}

    void ClientComponent::OnRequest(std::shared_ptr<c2s::Rpc::Call> t1)
    {
#ifdef __CLIENT_RPC_DEBUG_LOG__
        std::string json;
		LOG_INFO("========== call client ==========");
		LOG_INFO("func = " << t1->func());
		if(Helper::Proto::GetJson(t1->data(), json))
		{
			LOG_INFO("json = " << json);
		}
		LOG_INFO("=================================");
#endif

	}

    void ClientComponent::OnResponse(std::shared_ptr<c2s::Rpc::Response> t2)
    {
		auto iter = this->mRpcTasks.find(t2->rpc_id());
		if(iter != this->mRpcTasks.end())
		{
			iter->second->SetResult(t2);
			this->mRpcTasks.erase(iter);
			return;
		}
		LOG_ERROR("not find rpc task id = " << t2->rpc_id());
    }

    bool ClientComponent::LateAwake()
    {
        this->mTimerComponent = this->GetComponent<TimerComponent>();
		this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
		lua_State * luaState = this->mLuaComponent->GetLuaEnv();

		std::shared_ptr<Lua::Function> luaFunction = Lua::Function::Create(luaState, "Client", "Awake");
		if (luaFunction != nullptr)
		{
			luaFunction->Action();
		}
		return true;
    }

	void ClientComponent::OnAllServiceStart()
	{
		lua_State * luaState = this->mLuaComponent->GetLuaEnv();
		if(!Lua::Function::Get(luaState, "coroutine", "start"))
		{
			return;
		}
		Lua::lua_getfunction(luaState, "Client", "StartLogic");
		if (!lua_isfunction(luaState, -1))
		{
			return;
		}
		if (lua_pcall(luaState, 1, 0, 0) != 0)
		{
			LOG_ERROR(lua_tostring(luaState, -1));
		}
	}

	std::shared_ptr<c2s::Rpc::Response> ClientComponent::Call(std::shared_ptr<c2s::Rpc::Request> request)
	{
		std::shared_ptr<TaskSource<std::shared_ptr<c2s::Rpc_Response>>> rpcTask
				= std::make_shared<TaskSource<std::shared_ptr<c2s::Rpc_Response>>>();
		request->set_rpc_id(rpcTask->GetTaskId());
		this->mRpcTasks.emplace(rpcTask->GetTaskId(), rpcTask);
		this->mTcpClient->SendToServer(request);
		return rpcTask->Await();
	}

	void ClientComponent::OnTimeout(long long rpcId)
	{
//		auto iter = this->mRpcTasks.find(rpcId);
//		if (iter != this->mRpcTasks.end())
//		{
//			auto rpcTask = iter->second;
//			this->mRpcTasks.erase(iter);
//			rpcTask->SetResult(nullptr);
//		}
	}

	bool ClientComponent::StartConnect(const std::string& ip, unsigned short port)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& thread = this->GetApp()->GetTaskScheduler();
#else
		IAsioThread& thread = this->GetComponent<NetThreadComponent>()->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(thread, ip, port));
		this->mTcpClient = std::make_shared<TcpRpcClientContext>(socketProxy, this);
		if(this->mTcpClient->ConnectAsync()->Await())
		{
			this->mTcpClient->StartReceive();
			return true;
		}
		return false;
	}

	void ClientComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<ClientComponent>();
		luaRegister.PushExtensionFunction("Call", Lua::ClientEx::Call);
		luaRegister.PushExtensionFunction("StartConnectAsync", Lua::ClientEx::StartConnect);
	}
}

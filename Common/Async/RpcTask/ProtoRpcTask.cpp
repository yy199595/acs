#include "ProtoRpcTask.h"
#include<Core/App.h>

#include<Pool/MessagePool.h>
#include<Scene/LuaScriptComponent.h>
#include"ServerRpc/ProtoRpcComponent.h"
#ifdef __DEBUG__
#include"Scene/RpcConfigComponent.h"

#endif
namespace GameKeeper
{
    ProtoRpcTask::ProtoRpcTask(int method, long long rpcId)
        :  mMethod(method), mRpcId(rpcId), mTimerId(0)
    {
#ifdef __DEBUG__
        auto configComponent = App::Get().GetComponent<RpcConfigComponent>();
        auto config = configComponent->GetProtocolConfig(this->mMethod);
        LOG_WARN("create new rpc task : [" << config->Service << "."
                                           << config->Method << "] taskId = [" << rpcId << "]");
#endif
    }

    ProtoRpcTask::ProtoRpcTask(XCode code)
        : AsyncTask(code), mMethod(0), mTimerId(0), mRpcId(0)
    {

    }

    void ProtoRpcTask::OnTaskAwait()
    {
        auto rpcComponent = App::Get().GetComponent<ProtoRpcComponent>();
        if (rpcComponent != nullptr)
        {
            this->mTimerId = rpcComponent->AddRpcTask(this->shared_from_this());
        }
    }

    void LuaProtoRpcTask::OnResponse(const com::Rpc_Response * response)
    {
        if(response == nullptr)
        {

        }
        lua_pushinteger(this->mCoroutine, (int) response->code());
        if ((XCode) response->code() == XCode::Successful)
        {
            std::string json;
            Message *responseMessage = MessagePool::New(response->data());
            LOG_CHECK_RET(responseMessage && util::MessageToJsonString(*responseMessage, &json).ok());

            auto scriptCom = App::Get().GetComponent<LuaScriptComponent>();
            lua_getref(this->luaEnv, scriptCom->GetLuaRef("JsonRpc", "ToObject"));
            if (lua_isfunction(this->luaEnv, -1))
            {
                lua_pushlstring(this->luaEnv, json.c_str(), json.size());
                if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
                {
                    LOG_ERROR(lua_tostring(this->luaEnv, -1));
                    return;
                }
                lua_xmove(this->luaEnv, this->mCoroutine, 1);
                lua_presume(this->mCoroutine, this->luaEnv, 2);
                return;
            }
        }
        lua_presume(this->mCoroutine, this->luaEnv, 1);
    }
}


namespace GameKeeper
{
    CppProtoRpcTask::CppProtoRpcTask(XCode code)
        : ProtoRpcTask(code), mMessage(nullptr)
    {

    }

    CppProtoRpcTask::CppProtoRpcTask(int methodId, long long rpcId)
        : ProtoRpcTask(methodId, rpcId), mMessage(nullptr)
    {

    }

    CppProtoRpcTask::~CppProtoRpcTask()
    {

    }

    void CppProtoRpcTask::OnResponse(const com::Rpc_Response  * backData)
    {		
        if(backData == nullptr)
        {
            this->mTaskState = TaskTimeout;
            this->mCode = XCode::CallTimeout;
#ifdef __DEBUG__
			int methodId = this->GetMethodId();
			auto configComponent = App::Get().GetComponent<RpcConfigComponent>();
			const ProtocolConfig * config = configComponent->GetProtocolConfig(methodId);
			LOG_ERROR(config->Service << "." << config->Method << " call time out");
#endif // __DEBUG__
        }
        else if(this->mTaskState == TaskAwait)
        {
            this->mTaskState = TaskFinish;
            this->mCode = (XCode) backData->code();
            if(this->mCode == XCode::Successful && backData->has_data())
            {
				Message * message = MessagePool::NewByData(backData->data(), true);
				if (message != nullptr)
				{
                    this->mMessage = std::shared_ptr<Message>(message);
				}              
            }
        }
        this->RestoreAsyncTask();
    }

    XCode CppProtoRpcTask::Await()
    {
        this->AwaitTask();
        return this->mCode;
    }

    XCode CppProtoRpcTask::Await(std::shared_ptr<Message> response)
    {
        this->mMessage = std::move(response);
        return this->Await();
    }
}

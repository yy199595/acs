#include "RpcTask.h"
#include<Core/App.h>
#include"Rpc/RpcComponent.h"
#include<Pool/MessagePool.h>
#include<Scene/LuaScriptComponent.h>

#ifdef __DEBUG__
#include"Scene/RpcConfigComponent.h"
#endif
namespace GameKeeper
{
    RpcComponent * RpcTaskBase::mRpcComponent = nullptr;
    RpcTaskBase::RpcTaskBase(int method)
            : mMethod(method)
    {
        this->mCode = XCode::Successful;
        this->mState = TaskState::TaskReady;
        this->mTaskId = Helper::Guid::Create();
        this->mTaskComponent = App::Get().GetTaskComponent();
        if(mRpcComponent == nullptr)
        {
            mRpcComponent = App::Get().GetComponent<RpcComponent>();
            LOG_CHECK_RET(mRpcComponent);
        }
    }

    RpcTaskBase::RpcTaskBase(XCode code) : mMethod(0)
    {
        this->mCode = code;
        this->mState = TaskState::TaskFinish;
        this->mTaskId = Helper::Guid::Create();
        this->mTaskComponent = App::Get().GetTaskComponent();
        if(mRpcComponent == nullptr)
        {
            mRpcComponent = App::Get().GetComponent<RpcComponent>();
            LOG_CHECK_RET(mRpcComponent);
        }
    }

    bool RpcTaskBase::SetResult(const Rpc_Response *result)
    {
        if(this->mState == TaskState::TaskAwait)
        {
            if (result == nullptr)
            {
                this->mCode = XCode::CallTimeout;
#ifdef __DEBUG__
                int methodId = this->GetMethodId();
                auto configComponent =
                        App::Get().GetComponent<RpcConfigComponent>();
                const ProtocolConfig *config = configComponent->GetProtocolConfig(methodId);
                LOG_ERROR(config->Service << "." << config->Method << " call time out");
#endif // __DEBUG__
                this->mTaskComponent->Resume(this->mCorId);
                return true;
            }
            this->OnResponse(result);
            this->mTaskComponent->Resume(this->mCorId);
            return true;
        }
        return false;
    }

    void RpcTaskBase::AwaitTask()
    {
        if(this->mState == TaskState::TaskReady)
        {
            this->mState = TaskState::TaskAwait;
            mRpcComponent->AddRpcTask(this->shared_from_this());
#ifdef __DEBUG__
            int methodId = this->GetMethodId();
            auto configComponent =
                    App::Get().GetComponent<RpcConfigComponent>();
            const ProtocolConfig *config = configComponent->GetProtocolConfig(methodId);
            LOG_ERROR("await rpc [" << config->Service << "." << config->Method << "]");
#endif
            this->mTaskComponent->Yield(this->mCorId);
        }
    }

    XCode RpcTaskBase::GetCode()
    {
        this->AwaitTask();
        return this->mCode;
    }
}

namespace GameKeeper
{
    void LuaRpcTask::OnResponse(const Rpc_Response * response)
    {
        if(response == nullptr)
        {

        }
        lua_pushinteger(this->mCoroutine, (int) response->code());
        if ((XCode) response->code() == XCode::Successful)
        {
            std::string json;
            Message *responseMessage = Helper::Proto::New(response->data());
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
    RpcTask::RpcTask(XCode code)
        : RpcTaskBase(code)
    {

    }

    RpcTask::RpcTask(int methodId)
        : RpcTaskBase(methodId)
    {

    }

    void RpcTask::OnResponse(const com::Rpc_Response  * backData)
    {
        this->mCode = (XCode) backData->code();
        if (this->mCode == XCode::Successful && backData->has_data())
        {
            Message *message = Helper::Proto::NewByData(backData->data(), true);
            if (message != nullptr) {
                this->mMessage = std::shared_ptr<Message>(message);
            }
        }
    }
}

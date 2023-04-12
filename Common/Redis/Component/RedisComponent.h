﻿#pragma once

#include"Util/Guid/Guid.h"
#include"Redis/Config/RedisConfig.h"
#include"Redis/Client/TcpRedisClient.h"
#include"Rpc/Component/RpcTaskComponent.h"

namespace Tendo
{
	class ThreadComponent;

    class RedisComponent final : public RpcTaskComponent<int, RedisResponse>,
								 public ILuaRegister, public IRpc<RedisResponse>, public IDestroy
	{
	 public:
		RedisComponent() = default;
    public:
		bool Ping(size_t index);
		bool Send(const std::shared_ptr<RedisRequest>& request);
		bool Send(const std::shared_ptr<RedisRequest>& request, int & id);
		inline const RedisClientConfig & Config() const { return this->mConfig.Config(); }
	public:
        template<typename ... Args>
        bool Send(const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> Run(const std::string & cmd, Args&& ... args);
        template<typename ... Args> //同步命令
        std::shared_ptr<RedisResponse> SyncRun(const std::string& cmd, Args&& ... args);
    public:
        std::shared_ptr<RedisResponse> Run(const std::shared_ptr<RedisRequest>& request);
        std::shared_ptr<RedisResponse> SyncRun(const std::shared_ptr<RedisRequest>& request);
    private:
		TcpRedisClient * GetClient(size_t index = -1);
		void OnConnectSuccessful(const std::string &address) final;
        void OnMessage(std::shared_ptr<RedisResponse> message) final;
		TcpRedisClient * MakeRedisClient(const RedisClientConfig & config);
    private:
        bool Awake() final;
		void OnDestroy() final;
        bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	private:
		RedisConfig mConfig;
        std::shared_ptr<TcpRedisClient> mMainClient;
        std::vector<std::shared_ptr<TcpRedisClient>> mRedisClients;
    };

    template<typename ... Args>
    std::shared_ptr<RedisResponse> RedisComponent::Run(const std::string & cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(request);
    }

    template<typename ...Args>
    inline std::shared_ptr<RedisResponse> RedisComponent::SyncRun(const std::string& cmd, Args && ...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->mMainClient->SyncCommand(request);
    }

    template<typename ... Args>
    bool RedisComponent::Send(const std::string &cmd, Args &&...args)
    {
        TcpRedisClient * redisClient = this->GetClient();
        if(redisClient == nullptr)
        {
            return false;
        }
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
		redisClient->Send(request);
        return true;
    }
}
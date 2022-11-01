﻿#pragma once

#include"Guid/Guid.h"
#include"Json/JsonWriter.h"
#include"Timer/ElapsedTimer.h"
#include"Client/TcpRedisClient.h"
#include"Component/RpcTaskComponent.h"

namespace Sentry
{
	class NetThreadComponent;

    class RedisComponent final : public RpcTaskComponent<long long, RedisResponse>,
								 public ILuaRegister, public IStart, public IRpc<RedisResponse>
	{
	 public:
		RedisComponent() = default;
    public:
        bool Ping(TcpRedisClient * redisClient);
        TcpRedisClient * GetClient(const std::string & db);
    public:
        template<typename ... Args>
        bool Send(const std::string & name, const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> Run(const std::string & name, const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> Run(TcpRedisClient * c, const std::string & cmd, Args&& ... args);
    public:
        std::shared_ptr<RedisResponse> Run(const std::string & db, std::shared_ptr<RedisRequest> request);
        std::shared_ptr<RedisResponse> Run(TcpRedisClient * c, std::shared_ptr<RedisRequest> request);
    private:
		void OnConnectSuccessful(const std::string &address) final;
		TcpRedisClient * MakeRedisClient(const RedisClientConfig & config);
		void OnMessage(const std::string &address, std::shared_ptr<RedisResponse> message) final;
    private:
        bool Awake() final;
        bool Start() final;
        bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
    private:
        std::unordered_map<std::string, std::vector<std::shared_ptr<TcpRedisClient>>> mRedisClients;
    };

    template<typename ... Args>
    std::shared_ptr<RedisResponse> RedisComponent::Run(const std::string &name, const std::string & cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(name, request);
    }
    template<typename ... Args>
    std::shared_ptr<RedisResponse> RedisComponent::Run(TcpRedisClient * c, const std::string &cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(c, request);
    }
    template<typename ... Args>
    bool RedisComponent::Send(const std::string &name, const std::string &cmd, Args &&...args)
    {
        TcpRedisClient * redisClient = this->GetClient(name);
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
#pragma once

#include"Util/Guid/Guid.h"
#include"Core/Queue/Queue.h"
#include"Redis/Client/Client.h"
#include"Redis/Config/RedisConfig.h"
#include"Rpc/Component/RpcTaskComponent.h"

namespace joke
{
	struct RedisLuaData
	{
		std::string name;
		std::string json;
	};
}

namespace joke
{
	class ThreadComponent;

	class RedisComponent final : public RpcTaskComponent<int, redis::Response>,
		public ILuaRegister, public IRpc<redis::Request, redis::Response>, public IDestroy, public IServerRecord
	{
	 public:
		RedisComponent() = default;
    public:
		bool Ping();
		void Send(std::unique_ptr<redis::Request> request);
		void Send(std::unique_ptr<redis::Request> request, int & id);
		inline const redis::Config & Config() const { return this->mConfig; }
	public:
		bool UnLock(const std::string & key);
		bool Lock(const std::string & key, int timeout = 5);
	public:
        template<typename ... Args>
        void Send(const std::string & cmd, Args&& ... args);
        template<typename ... Args>
		redis::Response * Run(const std::string & cmd, Args&& ... args);
        template<typename ... Args> //同步命令
        std::unique_ptr<redis::Response> SyncRun(const std::string& cmd, Args&& ... args);
	public:
		bool Send(const RedisLuaData & data);
		bool Send(const RedisLuaData & data, int & taskId);
		redis::Response * CallLua(const RedisLuaData & data);
		std::unique_ptr<json::r::Document> Call(const RedisLuaData & data);
	public:
		bool Del(const std::string & key);
		redis::Response * Run(std::unique_ptr<redis::Request> request);
        std::unique_ptr<redis::Response> SyncRun(std::unique_ptr<redis::Request> request);
		bool MakeLuaRequest(const RedisLuaData & data, std::unique_ptr<redis::Request>& request);
	private:
		void Send(int id, std::unique_ptr<redis::Request> request);
		void OnMessage(int id, redis::Request * request, redis::Response * response) final;
	private:
		bool LoadRedisScript(const std::string & dir);
		bool OnLoadScript(const std::string & name, const std::string &md5);
	private:
        bool Awake() final;
		void OnDestroy() final;
        bool LateAwake() final;
		void OnRecord(json::w::Document &document) final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
	private:
		redis::Config mConfig;
		math::NumberPool<int> mNumPool;
		custom::Queue<int> mFreeClients; //空闲客户端
		std::unordered_map<int, redis::Client *> mClients;
		std::unordered_map<std::string, std::string> mLuaMap;
		std::queue<std::unique_ptr<redis::Request>> mRequests;
	};

    template<typename ... Args>
	redis::Response * RedisComponent::Run(const std::string & cmd, Args &&...args)
    {
        std::unique_ptr<redis::Request> request = std::make_unique<redis::Request>();
		{
			request->SetCommand(cmd);
			redis::Request::InitParameter(request.get(), std::forward<Args>(args)...);
		}
        return this->Run(std::move(request));
    }

    template<typename ...Args>
    inline std::unique_ptr<redis::Response> RedisComponent::SyncRun(const std::string& cmd, Args && ...args)
    {
        std::unique_ptr<redis::Request> request = std::make_unique<redis::Request>();
		{
			request->SetCommand(cmd);
			redis::Request::InitParameter(request.get(), std::forward<Args>(args)...);
		}
        return this->SyncRun(std::move(request));
    }

    template<typename ... Args>
	void RedisComponent::Send(const std::string &cmd, Args &&...args)
    {
		std::unique_ptr<redis::Request> request = redis::Request::Make(cmd, std::forward<Args>(args)...);
		{
			this->Send(std::move(request));
		}
    }
}
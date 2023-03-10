
#pragma once

#include"Component/RpcTaskComponent.h"
namespace Http
{
    class Response;
}
namespace Sentry
{
    class HttpRequestClient;
	class HttpComponent : public RpcTaskComponent<long long, Http::Response>, public ILuaRegister
	{
	 public:
		HttpComponent() = default;
	 public:
		std::shared_ptr<HttpRequestClient> CreateClient();
		int Download(const std::string & url, const std::string & path);
		std::shared_ptr<Http::Response> Get(const std::string& url, float second = 15.0f);
		std::shared_ptr<Http::Response> Post(const std::string& url, const std::string& data, float second = 15.0f);
    private:
        bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	 private:
		class TaskComponent* mTaskComponent;
        class ThreadComponent * mNetComponent;
        std::queue<std::shared_ptr<HttpRequestClient>> mClientPools;
	};
}
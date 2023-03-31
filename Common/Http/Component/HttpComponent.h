
#pragma once
#include"Util/Guid/NumberBuilder.h"
#include"Rpc/Component/RpcTaskComponent.h"
namespace Http
{
	class Request;
	class Response;
}
namespace Sentry
{
    class HttpRequestClient;
	class HttpComponent : public RpcTaskComponent<int, Http::Response>, public ILuaRegister
	{
	 public:
		HttpComponent();
	 public:
		std::shared_ptr<HttpRequestClient> CreateClient();
		int Download(const std::string & url, const std::string & path);
		std::shared_ptr<Http::Response> Get(const std::string& url, float second = 15.0f);
		std::shared_ptr<Http::Response> Post(const std::string& url, const std::string& data, float second = 15.0f);
	public:
		void Send(const std::shared_ptr<Http::Request> & request, int & taskId);
    private:
        bool LateAwake() final;
		void OnTaskComplete(int key) final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
		std::shared_ptr<Http::Response> Request(const std::shared_ptr<Http::Request> & request);
	 private:
        class ThreadComponent * mNetComponent;
		class AsyncMgrComponent* mTaskComponent;
		Util::NumberBuilder<int, 10> mNumberPool;
		std::queue<std::shared_ptr<HttpRequestClient>> mClientPools;
		std::unordered_map<int, std::shared_ptr<HttpRequestClient>> mUseClients;
	};
}
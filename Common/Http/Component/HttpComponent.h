
#pragma once
#include"Json/JsonWriter.h"
#include"Json/JsonReader.h"
#include"Component/RpcTaskComponent.h"
namespace Sentry
{
	class HttpServiceMethod;
	class HttpHandlerClient;
	class HttpAsyncResponse;
	class HttpHandlerRequest;
	class HttpRequestClient;
    class HttpHandlerResponse;
	class HttpComponent : public RpcTaskComponent<HttpAsyncResponse>, public ILuaRegister
	{
	 public:
		HttpComponent() = default;
	 public:
		std::shared_ptr<HttpRequestClient> CreateClient();
		XCode Download(const std::string & url, const std::string & path);
		std::shared_ptr<HttpAsyncResponse> Get(const std::string& url, float second = 15.0f);
		std::shared_ptr<HttpAsyncResponse> Post(const std::string& url, const std::string& data, float second = 15.0f);
    private:
        bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	 private:
		TimerComponent * mTimeComponent;
		class TaskComponent* mTaskComponent;
        class NetThreadComponent * mNetComponent;
        std::queue<std::shared_ptr<HttpRequestClient>> mClientPools;
        std::set<std::shared_ptr<HttpRequestClient>> mRequestClients;
	};
}
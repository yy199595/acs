
#pragma once
#include"Util/Guid/NumberBuilder.h"
#include"Rpc/Component/RpcTaskComponent.h"
namespace Http
{
	class Request;
	class IResponse;
	class DataResponse;
}
namespace Tendo
{
    class HttpRequestClient;
	class HttpComponent : public RpcTaskComponent<int, Http::IResponse>, public ILuaRegister
	{
	 public:
		HttpComponent();
	 public:
		std::shared_ptr<HttpRequestClient> CreateClient();
		bool Download(const std::string & url, const std::string & path);
		std::shared_ptr<Http::DataResponse> Get(const std::string& url, float second = 15.0f);
		std::shared_ptr<Http::DataResponse> Post(const std::string& url, const std::string& data, float second = 15.0f);
	public:
		std::shared_ptr<Http::DataResponse> Request(const std::shared_ptr<Http::Request>& request);
		void Send(const std::shared_ptr<Http::Request> & request, std::shared_ptr<Http::IResponse> response, int & taskId);
    private:
        bool LateAwake() final;
		void OnTaskComplete(int key) final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	 private:
        class ThreadComponent * mNetComponent;
		Util::NumberBuilder<int, 10> mNumberPool;
		std::queue<std::shared_ptr<HttpRequestClient>> mClientPools;
		std::unordered_map<int, std::shared_ptr<HttpRequestClient>> mUseClients;
	};
}
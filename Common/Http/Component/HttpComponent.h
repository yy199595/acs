
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
		bool Download(const std::string & url, const std::string & path, bool async = true);
		std::shared_ptr<Http::DataResponse> Get(const std::string& url, bool async = true, int second = 15);
		std::shared_ptr<Http::DataResponse> Post(const std::string& url, const std::string& data, bool async = true, int second = 15);
	public:
		std::shared_ptr<Http::DataResponse> Await(const std::shared_ptr<Http::Request>& request);
		bool Send(const std::shared_ptr<Http::Request> & request, std::shared_ptr<Http::IResponse> & response); // 同步发送
		bool Send(const std::shared_ptr<Http::Request> & request, std::shared_ptr<Http::IResponse> & response, int & taskId); // 异步发送
    private:
        bool LateAwake() final;
		void OnTaskComplete(int key) final;
		std::shared_ptr<HttpRequestClient> CreateClient();
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	 private:
        class ThreadComponent * mNetComponent;
		std::queue<std::shared_ptr<HttpRequestClient>> mClientPools;
		std::unordered_map<int, std::shared_ptr<HttpRequestClient>> mUseClients;
	};
}
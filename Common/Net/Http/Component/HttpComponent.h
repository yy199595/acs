
#pragma once
#ifdef __ENABLE_OPEN_SSL__
#include"asio/ssl.hpp"
#endif
#include"Core/Map/HashMap.h"
#include"Core/Pool/ArrayPool.h"
#include"Rpc/Component/RpcTaskComponent.h"
namespace http
{
	class Data;
	class Request;
	class Response;
	class RequestClient;
}
namespace joke
{
	class HttpComponent : public RpcTaskComponent<int, http::Response, false>,
			public ILuaRegister, public IRpc<http::Request, http::Response>
	{
	 public:
		HttpComponent();
	 public:
		http::Response * Get(const std::string& url, int second = 15);
		int Download(const std::string & url, const std::string & path);
		http::Response * Post(const std::string& url, const std::string& data, int second = 15);
	public:
		http::Response * Do(std::unique_ptr<http::Request> request);
		http::Response * Do(std::unique_ptr<http::Request> request, std::unique_ptr<http::Data> body);
	public:
		int Send(std::unique_ptr<http::Request> request, std::function<void(http::Response*)> && cb);
		int Send(std::unique_ptr<http::Request> request, std::unique_ptr<http::Response> response, int & taskId); // 异步发送
	private:
#ifdef __ENABLE_OPEN_SSL__
		bool Awake() final;
#endif
        bool LateAwake() final;
		void OnTaskComplete(int key) final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
		http::RequestClient * CreateClient(http::Request * request);
		void OnMessage(http::Request *request, http::Response *response) final;
	private:
		math::NumberPool<int> mNumPool;
        class ThreadComponent * mNetComponent;
#ifdef __ENABLE_OPEN_SSL__
		std::unordered_map<std::string, asio::ssl::context *> mSslContexts;
#endif
		std::string mPemPath;
		custom::ArrayPool<http::RequestClient, 10> mClientPools;
		custom::HashMap<int, http::RequestClient *> mUseClients;
	};
}
#pragma once

#include<memory>
#include"ServiceHost.h"
#include"Message/c2s.pb.h"
#include"Json/JsonWriter.h"
#include"Component/Component.h"
#include"Global/ServiceConfig.h"
using namespace std;

namespace Sentry
{
	class ServiceMethod;
	class MessageRpcClient;
    class Service : public Component, public ILuaRegister, public ServiceHost,
							 public IService<com::rpc::request, com::rpc::response>
	{
	 public:
		Service();
		virtual ~Service() override = default;
	 public:
		XCode Send(const std::string& func, const Message & message);
		XCode Send(long long userId, const std::string& func, const Message & message);
		XCode Send(const std::string & address, const std::string& func, const Message & message);
	 public:
		XCode Call(const std::string& address, const std::string& func);
		XCode Call(const std::string& address, const std::string& func, const Message& message);
		XCode Call(const std::string& address, const std::string& func, std::shared_ptr<Message> response);
		XCode Call(const std::string& address, const std::string& func, const Message& message, std::shared_ptr<Message> response);
	 public:
		XCode Call(long long userId, const std::string& func);
		XCode Call(long long userId, const std::string& func, const Message& message);
		XCode Call(long long userId, const std::string& func, std::shared_ptr<Message> response);
        XCode Call(long long userId, std::shared_ptr<com::rpc::request> request, std::shared_ptr<Message> response);
        XCode Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response);
		XCode Call(const std::string& address, std::shared_ptr<com::rpc::request> request, std::shared_ptr<Message> response);
	public:
        bool IsStartComplete() final{return this->GetHostSize() > 0; };
        const RpcServiceConfig & GetServiceConfig() { return *this->mConfig; }
        const std::string & GetLocalHost() final { return this->mLocalAddress; }
		XCode SendRequest(const std::string& address, std::shared_ptr<com::rpc::request> request);
		std::shared_ptr<com::rpc::request> NewRpcRequest(const std::string& func, long long userId);
		std::shared_ptr<com::rpc::request> NewRpcRequest(const std::string& func, long long userId, const Message& message);
	 protected:
		bool LateAwake() override;
		bool LoadConfig(const rapidjson::Value & json);
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) override;
	private:
		RpcServiceConfig * mConfig;
        std::string mLocalAddress;
        class TcpRpcComponent* mRpcComponent;
        std::vector<std::string> mServiceHosts;
        class RpcClientComponent* mClientComponent;
	};
}
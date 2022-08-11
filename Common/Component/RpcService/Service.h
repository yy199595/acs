#pragma once

#include<memory>
#include"Message/c2s.pb.h"
#include"Json/JsonWriter.h"
#include"AddressProxy.h"
#include"Component/Component.h"
#include"Global/ServiceConfig.h"
using namespace std;

namespace Sentry
{
	class ServiceMethod;
	class MessageRpcClient;
	class Service : public Component, public ILuaRegister,
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
		XCode Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response);
		XCode Call(long long userId, std::shared_ptr<com::rpc::request> request, std::shared_ptr<Message> response);
		XCode Call(const std::string& address, std::shared_ptr<com::rpc::request> request, std::shared_ptr<Message> response);
	public:
		AddressProxy & GetAddressProxy() { return this->mAddressProxy;}
		const RpcServiceConfig & GetServiceConfig() { return *this->mConfig; }
		bool IsStartComplete() final{return this->mAddressProxy.GetSize() > 0; };
		XCode SendRequest(const std::string& address, std::shared_ptr<com::rpc::request> request);
		std::shared_ptr<com::rpc::request> NewRpcRequest(const std::string& func, long long userId);
		std::shared_ptr<com::rpc::request> NewRpcRequest(const std::string& func, long long userId, const Message& message);
	 protected:
		bool LateAwake() override;
		bool LoadConfig(const rapidjson::Value & json);
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) override;
	private:
		std::string mLocalAddress;
		RpcServiceConfig * mConfig;
        AddressProxy mAddressProxy;
        std::vector<std::string> mAllAddress;
		class TcpRpcComponent* mRpcComponent;
		class RpcClientComponent* mClientComponent;
	};
}
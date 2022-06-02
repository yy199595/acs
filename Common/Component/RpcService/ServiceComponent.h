#pragma once

#include<memory>
#include"Protocol/c2s.pb.h"
#include"Json/JsonWriter.h"
#include"AddressProxy.h"
#include"Component/Component.h"
using namespace std;

namespace Sentry
{
	class ServiceMethod;
	class ServerClientContext;
	class ServiceComponent : public Component, public ILuaRegister,
							 public IService<com::Rpc::Request, com::Rpc::Response>
	{
	 public:
		ServiceComponent() = default;
		virtual ~ServiceComponent() override = default;
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
		XCode Call(long long userId, std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<Message> response);
		XCode Call(const std::string& address, std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<Message> response);
	public:
		bool SocketIsOpen(const std::string & address);
		AddressProxy & GetAddressProxy() { return this->mAddressProxy;}
		bool IsStartComplete() final{return this->mAddressProxy.GetSize() > 0; };
		XCode SendRequest(const std::string& address, std::shared_ptr<com::Rpc::Request> request);
		std::shared_ptr<com::Rpc::Request> NewRpcRequest(const std::string& func, long long userId);
		std::shared_ptr<com::Rpc::Request> NewRpcRequest(const std::string& func, long long userId, const Message& message);
	 protected:
		bool LateAwake() override;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) override;
	private:
		std::string mLocalAddress;
		AddressProxy mAddressProxy;
		std::vector<std::string> mAllAddress;
		class RpcHandlerComponent* mRpcComponent;
		class MainRedisComponent* mRedisComponent;
		class RpcClientComponent* mClientComponent;
	};
}
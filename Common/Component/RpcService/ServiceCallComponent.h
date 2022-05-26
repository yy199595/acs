#pragma once

#include<memory>
#include"Protocol/c2s.pb.h"
#include"Json/JsonWriter.h"
#include"Component/Component.h"
using namespace std;
using namespace com;

namespace Sentry
{
	class ServiceMethod;
	class ServerClientContext;
	class ServiceCallComponent : public Component, public ILuaRegister
	{
	 public:
		ServiceCallComponent() = default;
		virtual ~ServiceCallComponent() override = default;
	 public:
		XCode Send(const std::string& func, const Message & message);
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
		size_t GetAddressSize();
		bool AllotAddress(std::string & address);
		bool HasAddress(const std::string & address);
		bool AddAddress(const std::string & address);
		bool DelAddress(const std::string & address);
	public:
		bool DelUserAddress(long long id);
		bool GetUserAddress(long long id, std::string& address);
		bool AddUserAddress(long long id, const std::string & address);
	protected:
		bool LateAwake() override;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) override;
		XCode SendRequest(const std::string& address, std::shared_ptr<com::Rpc::Request> request);
		std::shared_ptr<com::Rpc::Request> NewRpcRequest(const std::string& func, long long userId, const Message* message);
	private:
		std::string mLocalAddress;
		std::vector<std::string> mAllAddress;
		class UserSyncComponent * mSyncComponent;
		class RpcHandlerComponent* mRpcComponent;
		class MainRedisComponent* mRedisComponent;
		class RpcClientComponent* mClientComponent;
		std::unordered_map<long long, std::string> mUserAddress;
	};
}
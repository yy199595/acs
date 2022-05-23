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
	class ServiceCallComponent : public Component
	{
	 public:
		ServiceCallComponent() = default;
		virtual ~ServiceCallComponent() override = default;
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
	 public:
		bool LateAwake() override;
		XCode PublishEvent(const std::string& eveId);
		XCode PublishEvent(const std::string& eveId, Json::Writer& message);
		virtual bool GetEntityAddress(long long id, std::string& address) = 0;
		XCode Call(const std::string& address, std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<Message> response);
	 private:
		XCode SendRequest(const std::string& address, std::shared_ptr<com::Rpc::Request> request);
		std::shared_ptr<com::Rpc::Request> NewRpcRequest(const std::string& func, long long userId, const Message* message);
	 protected:
		std::string mLocalAddress;
		class RpcHandlerComponent* mRpcComponent;
		class MainRedisComponent* mRedisComponent;
		class RpcClientComponent* mClientComponent;
	};
}
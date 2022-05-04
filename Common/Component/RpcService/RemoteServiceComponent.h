#pragma once

#include<memory>
#include"Protocol/c2s.pb.h"
#include"Component/Component.h"
using namespace std;
using namespace com;

namespace Sentry
{
	class ServiceMethod;
	class ServerRpcClientContext;
	class RemoteServiceComponent : public Component
	{
	 public:
		RemoteServiceComponent() = default;
		~RemoteServiceComponent() override = default;
	 public:
		virtual XCode Call(const std::string& address, const std::string& func);

		virtual XCode Call(const std::string& address, const std::string& func, const Message& message);

		virtual XCode Call(const std::string& address, const std::string& func, std::shared_ptr<Message> response);

		virtual XCode Call(const std::string& address, const std::string& func, const Message& message, std::shared_ptr<Message> response);

	 public:

		virtual XCode Call(const std::string& func, long long userId);

		virtual XCode Call(const std::string& func, long long userId, const Message& message);

		virtual XCode Call(const std::string& func, long long userId, std::shared_ptr<Message> response);

		virtual XCode Call(const std::string& func, long long userId, const Message& message, std::shared_ptr<Message> response);

	public:
		std::shared_ptr<com::Rpc::Response> StartCall(const std::string& address, std::shared_ptr<com::Rpc::Request> request);
	 protected:
		bool LateAwake() override;
		virtual bool GetEntityAddress(long long id, std::string& address) = 0;
		virtual XCode SendRequest(const std::string & address, std::shared_ptr<com::Rpc::Request> request) = 0;
		std::shared_ptr<com::Rpc::Request> NewRpcRequest(const std::string& func, long long userId, const Message* message);
	 protected:
		std::string mLocalAddress;
		class RpcComponent* mRpcComponent;
	};
}
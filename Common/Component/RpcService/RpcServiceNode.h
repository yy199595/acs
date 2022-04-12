#pragma once

#include<memory>
#include"Protocol/c2s.pb.h"
#include"Component/Component.h"
using namespace std;
using namespace com;

namespace Sentry
{
	class ServiceRecord
	{
	 public:
		ServiceRecord();
	 public:
		long long GetWeight();
		void OnCall(long long ms);
	 private:
		long long mCallCount;
		long long mStartTime;
	};
}

namespace Sentry
{
	class ServiceMethod;
	class ProtoRpcClient;
	class RpcServiceNode : public Component, public IServiceBase
	{
	 public:
		RpcServiceNode() = default;

		~RpcServiceNode() override = default;
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

	 protected:
		bool LateAwake() override;
		std::shared_ptr<com::Rpc::Response> StartCall(const std::string& address, std::shared_ptr<com::Rpc::Request> request);
		std::shared_ptr<com::Rpc::Request> NewRpcRequest(const std::string& func, long long userId, const Message* message);

	protected:
		void OnAddAddress(const std::string &address) final;
		void OnDelAddress(const std::string &address) final;
		bool IsStartComplete() final { return !this->mRemoteAddressList.empty(); }
	 public:
		void AddEntity(long long id);
		void DelEntity(long long id);
		bool AllotAddress(std::string& address);
	 protected:
		bool GetEntityAddress(long long id, std::string& address);
	 protected:
		class RpcComponent* mRpcComponent;
		std::set<std::string> mRemoteAddressList;
		class RpcClientComponent* mRpcClientComponent;
	 private:
		std::unordered_map<long long, std::string> mUserAddressMap;
	};
}
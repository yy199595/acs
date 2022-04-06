#pragma once

#include<memory>
#include"Protocol/c2s.pb.h"
#include"Service/ServiceNode.h"
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

	class RpcServiceBase : public Component
	{
	public:
		RpcServiceBase() = default;

		~RpcServiceBase() override = default;
	 public:
		virtual XCode Call(const std::string& func) = 0;

		virtual XCode Call(const std::string& func, const Message& message) = 0;

		virtual XCode Call(const std::string& func, std::shared_ptr<Message> response) = 0;

		virtual XCode Call(const std::string& func, const Message& message, std::shared_ptr<Message> response) = 0;

	 public:
		virtual XCode Call(const std::string& func, long long userId) = 0;

		virtual XCode Call(const std::string& func, long long userId, const Message& message) = 0;

		virtual XCode Call(const std::string& func, long long userId, std::shared_ptr<Message> response) = 0;

		virtual XCode Call(const std::string& func, long long userId, const Message& message, std::shared_ptr<Message> response) = 0;
	 protected:
		bool LateAwake() override;
	public:
		virtual void GetServiceName() = 0;
		virtual bool SwitchAddress(const std::string & address) = 0;
		virtual void AddNodeAddress(const std::string & address) = 0;
		virtual void DelNodeAddress(const std::string & address) = 0;
	 protected:
		class RpcClientComponent * mRpcClientComponent;
		class RpcConfigComponent * mRpcConfigComponent;
	};

}
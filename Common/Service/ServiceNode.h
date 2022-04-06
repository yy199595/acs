//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_SERVICENODE_H
#define SERVER_SERVICENODE_H
#include<string>
#include"XCode/XCode.h"
#include"Protocol/com.pb.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
namespace Sentry
{
	class ProtoConfig;
	class ServiceNode
	{
	public:
		ServiceNode(const std::string & name);
		ServiceNode(const std::string & name, const std::string & address);

	public:
		const std::string & GetAddress() { return this->mAddress;}
	public:
		XCode Call(const std::string& func);

		XCode Call(const std::string& func, const Message& message);

		XCode Call(const std::string& func, std::shared_ptr<Message> response);

		XCode Call(const std::string& func, const Message& message, std::shared_ptr<Message> response);

	public:
		XCode Call(const std::string& func, long long userId);

		XCode Call(const std::string& func, long long userId, const Message& message);

		XCode Call(const std::string& func, long long userId, std::shared_ptr<Message> response);

		XCode Call(const std::string& func, long long userId, const Message& message, std::shared_ptr<Message> response);

	private:
		std::shared_ptr<com::Rpc::Request> NewRequestData(const std::string & func);
		std::shared_ptr<com::Rpc::Response> StartCall(std::shared_ptr<com::Rpc::Request> request);
	private:
		std::string mIp;
		unsigned short mPort;
		std::string mService;
		std::string mAddress;
		class RpcComponent *mRpcComponent;
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
		class RpcConfigComponent *mRpcConfigComponent;
		class RpcClientComponent * mRpcClientComponent;
	};
}

#endif //SERVER_SERVICENODE_H

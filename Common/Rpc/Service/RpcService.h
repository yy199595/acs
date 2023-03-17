#pragma once

#include<memory>
#include<vector>
#include"Component/Component.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
namespace Rpc
{
    class Packet;
};
namespace Sentry
{
	class ServiceMethod;
	class InnerNetClient;
	class RpcService : public Component, public IServiceBase
	{
	public:
		RpcService() = default;
	public:
		const std::string& GetServer() const { return this->mCluster; }
	public:
		int Send(const std::string& func, const Message& message);
		int Send(const std::string& address, const std::string& func);
		int Send(const std::string& address, const std::string& func, const Message& message);
	public:
		int Send(long long userId, const std::string& func);
		int Send(long long userId, const std::string& func, const Message& message);
		int Send(const std::string & address, const std::string & func, long long userId, const Message * message = nullptr);
	public:
		int Call(const std::string& address, const std::string& func);
		int Call(const std::string& address, const std::string& func, const Message& message);
		int Call(const std::string& address, const std::string& func, std::shared_ptr<Message> response);
		int Call(const std::string& address, const std::string& func, const Message& message, std::shared_ptr<Message> response);
	public:
		int Call(long long userId, const std::string& func);
		int Call(long long userId, const std::string& func, const Message& message);
		int Call(long long userId, const std::string& func, std::shared_ptr<Message> response);
		int Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response);
	public:
		bool StartSend(long long userId, const std::string& func, const Message* message = nullptr);
		bool StartSend(const std::string& address, const std::string& func, const Message* message = nullptr);
		std::shared_ptr<Rpc::Packet> CallAwait(long long userId, const std::string& func, const Message* message = nullptr);
		std::shared_ptr<Rpc::Packet> CallAwait(const std::string& address, const std::string& func, const Message* message = nullptr);
	protected:
		bool LateAwake() override;
	public:
		bool RandomAddress(std::string& address);
		virtual int Invoke(const std::string& method, std::shared_ptr<Rpc::Packet> message) = 0;
	private:
		std::string mCluster;
		std::string mLocationAddress;
		std::vector<std::string> mServiceHosts;
		class NodeMgrComponent* mLocationComponent;
		class InnerNetMessageComponent* mMessageComponent;
	};
}
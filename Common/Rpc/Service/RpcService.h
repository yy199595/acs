#pragma once

#include<memory>
#include<vector>
#include"Entity/Component/Component.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
namespace Rpc
{
    class Packet;
};
namespace Tendo
{
	class ServiceMethod;
	class InnerNetTcpClient;
	class RpcService : public Component, public IServiceBase
	{
	public:
		RpcService();
	public:
		inline void SetProto(int proto) { this->mProto = proto; }
		inline void SetAsync(bool async) { this->mAsync = async; }
		inline void SetNetType(int type) { this->mNetType = type;}
		const std::string& GetServer() const { return this->mCluster; }
	public:
		int Send(const std::string& func, const Message& request);
		int Send(const std::string& address, const std::string& func);
		int Send(const std::string& address, const std::string& func, const Message& request);
	public:
		int Send(long long userId, const std::string& func);
		int Send(long long userId, const std::string& func, const Message& request);
		int Send(const std::string & address, const std::string & func, long long userId, const Message * request = nullptr);
	public:
		int Call(const std::string& address, const std::string& func);
		int Call(const std::string& address, const std::string& func, const Message& request);
		int Call(const std::string& address, const std::string& func, std::shared_ptr<Message> response);
		int Call(const std::string& address, const std::string& func, const Message& request, std::shared_ptr<Message> response);
	public:
		int Call(long long userId, const std::string& func);
		int Call(long long userId, const std::string& func, const Message& request);
		int Call(long long userId, const std::string& func, std::shared_ptr<Message> response);
		int Call(long long userId, const std::string& func, const Message& request, std::shared_ptr<Message> response);
	public:
		bool StartSend(long long userId, const std::string& func, const Message* request = nullptr);
		bool StartSend(const std::string& address, const std::string& func, const Message* request = nullptr);
		std::shared_ptr<Rpc::Packet> CallAwait(long long userId, const std::string& func, const Message* request = nullptr);
		std::shared_ptr<Rpc::Packet> CallAwait(const std::string& address, const std::string& func, const Message* request = nullptr);
	protected:
		bool LateAwake() final;
	public:
		virtual int Invoke(const std::string& method, std::shared_ptr<Rpc::Packet> message) = 0;
	private:
		int mProto;
		int mAsync;
		int mNetType;
		std::string mCluster;
		std::string mLocationAddress;
		std::vector<std::string> mServiceHosts;
		class HttpComponent * mHttpComponent;
		class InnerNetComponent* mTcpComponent;
        class NodeMgrComponent* mLocationComponent;
	};
}
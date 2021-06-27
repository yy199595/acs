#pragma once
#include<Object/Object.h>
#include<Service/ProxyService.h>
namespace SoEasy
{
	class LocalRetActionProxy;
	class ServiceNode : public Object
	{
	public:
		ServiceNode(int areaId, int nodeId, const std::string name, const std::string address, const std::string nAddress = "");
	public:
		const int GetAreaId() { return this->mAreaId; }
		const int GetNodeId() { return this->mNodeId; }
		const std::string & GetAddress() { return this->mAddress; }
		const std::string & GetNodeName() { return this->mNodeName; }
	public:
		bool AddService(const std::string & service);
		bool HasService(const std::string & service);
	public:
		void OnSystemUpdate();
	public:
		std::string GetJsonString();
		XCode Notice(const std::string & service, const std::string & method, const Message * request = nullptr); //不回应
	public: // c++ 使用
		XCode Call(const std::string & service, const std::string & method);
		XCode Call(const std::string & service, const std::string & method, Message & response);
		XCode Call(const std::string & service, const std::string & method, const Message * request);
		XCode Call(const std::string & service, const std::string & method, const Message * request, Message & response);
	public: // lua使用

	public:
		void PushMessageData(SharedPacket messageData);
		void PushMessageData(const std::string & service, const std::string & method, const Message * request = nullptr, shared_ptr<LocalRetActionProxy> rpcReply = nullptr);
	private:
		const int mAreaId;
		const int mNodeId;
		const std::string mAddress; //监听地址
		const std::string mNoticeAddress;	//通信地址
		const std::string mNodeName;	//进程名字
		SharedTcpSession mNodeTcpSession;
		std::set<std::string> mServiceArray;	//服务列表
		class CoroutineManager * mCorManager;	//协程
		class ActionManager * mActionManager;
		class NetWorkManager * mNetWorkManager;	
		class ServiceManager * mServiceManager;
		std::queue<shared_ptr<PB::NetWorkPacket>> mMessageQueue;
		char mSendSharedBuffer[ASIO_TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
	};
}
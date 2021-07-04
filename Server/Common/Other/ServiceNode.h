#pragma once
#include<Object/Object.h>
#include<Protocol/com.pb.h>
#include<Protocol/s2s.pb.h>
#include<Manager/ServiceNodeManager.h>
namespace SoEasy
{
	class LocalRetActionProxy;
	class ServiceNode : public Object
	{
	public:
		ServiceNode(int areaId, int nodeId, const std::string name, const std::string address, const std::string nAddress = "");
	public:
		const int GetAreaId() { return this->mNodeInfoMessage.areaid(); }
		const int GetNodeId() { return this->mNodeInfoMessage.nodeid(); }
		const std::string &GetAddress() { return this->mNodeInfoMessage.address(); }
		const std::string &GetNodeName() { return this->mNodeInfoMessage.servername(); }
		const s2s::NodeData_NodeInfo &GetNodeMessage() { return this->mNodeInfoMessage; }

	public:
		bool AddService(const std::string &service);
		bool HasService(const std::string &service);

	public:
		void OnSystemUpdate();
	public:
		std::string GetJsonString();
		XCode Notice(const std::string &service, const std::string &method); //不回应
		XCode Notice(const std::string &service, const std::string &method, const Message & request); //不回应
	public:
		XCode Invoke(const std::string &service, const std::string &method);
		XCode Invoke(const std::string &service, const std::string &method, const Message & request);
	public: // c++ 使用
		XCode Call(const std::string &service, const std::string &method, Message &response);
		XCode Call(const std::string &service, const std::string &method, const Message & request, Message &response);
	public: // lua使用
		void PushMessageData(PB::NetWorkPacket * messageData);
		void PushMessageData(const std::string &service, const std::string &method, const Message * request = nullptr, shared_ptr<LocalRetActionProxy> rpcReply = nullptr);
	private:
		std::string mIp;
		unsigned short mPort;
		const std::string mAddress;		  //监听地址
		const std::string mNoticeAddress; //通信地址
		const std::string mNodeName;	  //进程名字
		std::set<std::string> mServiceArray; //服务列表
		class CoroutineManager *mCorManager; //协程
		class ActionManager *mActionManager;
		class NetProxyManager *mNetWorkManager;
		s2s::NodeData_NodeInfo mNodeInfoMessage;
		ServiceNodeManager * mServiceNodeManager;
		std::queue<PB::NetWorkPacket *> mMessageQueue;
		char mSendSharedBuffer[ASIO_TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
	};
}
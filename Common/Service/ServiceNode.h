#pragma once


#include <Object/Object.h>
#include <Component/Service/ServiceNodeComponent.h>
namespace Sentry
{
	class PacketMapper;
    class LocalRetActionProxy;

    class ServiceNode : public Object
    {
    public:
        ServiceNode(int uid, const std::string name, const std::string address);
    public:
      
		const int GetNodeUId() { return this->mNodeUId; }

		const std::string &GetAddress() { return this->mAddress; }

		const std::string &GetNodeName() { return this->mNodeName; }

    public:
        bool AddService(const std::string &service);

        bool HasService(const std::string &service);
		void AddMessageToQueue(PacketMapper * message, bool yield = true);
    public:
		void OnConnectNodeAfter();
    public:
       
        XCode Notice(const std::string &service, const std::string &method);                        //不回应
        XCode Notice(const std::string &service, const std::string &method, const Message &request);//不回应
    public:
        XCode Invoke(const std::string &service, const std::string &method);

        XCode Invoke(const std::string &service, const std::string &method, const Message &request);

    public:// c++ 使用
        XCode Call(const std::string &service, const std::string &method, Message &response);

        XCode Call(const std::string &service, const std::string &method, const Message &request, Message &response);

	private:
		XCode SendRpcMessage(PacketMapper * message);
		XCode SendRpcMessage(PacketMapper * message, Message &response);
	private:
		void HandleMessageSend();
		
    private:
		int mNodeUId;
		bool mIsClose;
        std::string mIp;
		unsigned int mCorId;
        unsigned short mPort;	
        const std::string mAddress;         //监听地址
        const std::string mNodeName;        //进程名字
        std::set<std::string> mServiceArray;//服务列表
        class CoroutineComponent *mCorComponent;//协程
		class TcpProxySession * mTcpSession;
        class ActionComponent *mActionManager;
		std::queue<PacketMapper *> mNodeMessageQueue;
    };
}// namespace Sentry
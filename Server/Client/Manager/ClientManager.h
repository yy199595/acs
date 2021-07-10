#pragma once

#include<Manager/ScriptManager.h>
#include<Manager/NetProxyManager.h>
#include<Protocol/c2s.pb.h>
using namespace SoEasy;
using namespace google::protobuf;
namespace Client
{
	class ClientManager : public NetProxyManager, public IFrameUpdate
	{
	public:
		ClientManager() { }
		~ClientManager() { }
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
		void OnFrameUpdate(float t) override;
	public:
		XCode Notice(const std::string &service, const std::string &method); //不回应
		XCode Notice(const std::string &service, const std::string &method, const Message & request); //不回应
	public:
		XCode Invoke(const std::string &service, const std::string &method);
		XCode Invoke(const std::string &service, const std::string &method, const Message & request);
	public: // c++ 使用
		XCode Call(const std::string &service, const std::string &method, Message &response);
		XCode Call(const std::string &service, const std::string &method, const Message & request, Message &response);
	private:
		void InvokeAction();
	private:
		std::string mAddress;
		std::string mConnectIp;
		unsigned short mConnectPort;
		class ScriptManager * mScriptManager;
		CoroutineManager * mCoroutineManager;
	private:
		std::queue<PB::NetWorkPacket *> mWaitSendMessages;
		
	};
}
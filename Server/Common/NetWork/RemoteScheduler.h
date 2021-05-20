
#include<XCode/XCode.h>
#include<Protocol/Common.pb.h>
#include<NetWork/TcpClientSession.h>
#include<NetWork/NetWorkRetAction.h>
#include<Manager/ActionManager.h>
#include<Manager/NetWorkManager.h>
#include<Script/LuaTable.h>
namespace SoEasy
{
	class RemoteScheduler
	{
	public:
		RemoteScheduler(long long operaotrId = 0);
		RemoteScheduler(shared_ptr<TcpClientSession>, long long operId = 0);
		~RemoteScheduler() { }
	public:
		XCode Call(const std::string service, std::string func);
		XCode Call(const std::string service, std::string func, NetWorkRetAction1 action);
		template<typename T>
		XCode Call(const std::string service, std::string func, NetWorkRetAction2<T> action);
	public:
		XCode Call(const std::string service, std::string func, LuaTable & message);
		XCode Call(const std::string service, std::string func, LuaTable & message, NetLuaRetAction * action);
		XCode Call(const std::string service, std::string func, LuaTable & luaTable, NetLuaWaitAction * action);
	public:
		XCode Call(const std::string service, std::string func, Message * message);
		XCode Call(const std::string service, std::string func, Message * message, NetLuaRetAction * action);
		XCode Call(const std::string service, std::string func, Message * message, NetWorkRetAction1 action);
		XCode Call(const std::string service, std::string func, Message * message, NetLuaWaitAction * action);
		template<typename T>
		XCode Call(const std::string service, std::string func, Message * message, NetWorkRetAction2<T> action);
	private:	
		XCode SendCallMessage(const std::string & service, std::string & func, LuaTable & message, shared_ptr<LocalRetActionProxy> action = nullptr);
		XCode SendCallMessage(const std::string & service, std::string & func, Message * message = nullptr, shared_ptr<LocalRetActionProxy> action = nullptr);
	public:
		long long mOperatorId;
		std::string mMessageBuffer;
		std::string mBindSessionAdress;
		NetWorkManager * mNetWorkManager;
		ActionManager * mFunctionManager;
	};

	template<typename T>
	inline XCode RemoteScheduler::Call(const std::string service, std::string func, NetWorkRetAction2<T> action)
	{
		auto pAction = make_shared<LocalRetActionProxy2<T>>(action, func);
		return this->SendCallMessage(service, func, nullptr, pAction);
	}

	template<typename T>
	inline XCode RemoteScheduler::Call(const std::string service, std::string func, google::protobuf::Message * message, NetWorkRetAction2<T> action)
	{
		auto pAction = make_shared<LocalRetActionProxy2<T>>(action, func);
		return this->SendCallMessage(service, func, message, pAction);
	}
}
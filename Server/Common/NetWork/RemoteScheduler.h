
#include<XCode/XCode.h>
#include<Protocol/Common.pb.h>
#include<NetWork/TcpClientSession.h>
#include<NetWork/NetWorkRetAction.h>
#include<Manager/LocalActionManager.h>
#include<Manager/NetWorkManager.h>
#include<Script/LuaType/LuaTable.h>
namespace SoEasy
{
	class RemoteScheduler
	{
	public:
		RemoteScheduler(long long operaotrId = 0);
		RemoteScheduler(shared_ptr<TcpClientSession>, long long operId = 0);
		~RemoteScheduler() { }
	public:
		bool Call(std::string func);
		bool Call(std::string func, NetWorkRetAction1 action);
		template<typename T>
		bool Call(std::string func, NetWorkRetAction2<T> action);
	public:
		bool Call(std::string func, LuaTable & message);
		bool Call(std::string func, LuaTable & message, NetLuaRetAction * action);
		bool Call(std::string func, LuaTable & luaTable, NetLuaWaitAction * action);
	public:
		bool Call(std::string func, Message * message);
		bool Call(std::string func, Message * message, NetLuaRetAction * action);	
		bool Call(std::string func, Message * message, NetWorkRetAction1 action);
		bool Call(std::string func, Message * message, NetLuaWaitAction * action);
		template<typename T>
		bool Call(std::string func, Message * message, NetWorkRetAction2<T> action);
	private:	
		bool SendCallMessage(std::string & func, LuaTable & message, shared_ptr<LocalRetActionProxy> action = nullptr);
		bool SendCallMessage(std::string & func, Message * message = nullptr, shared_ptr<LocalRetActionProxy> action = nullptr);		
	public:
		template<typename T, typename ... Args>
		unsigned long long CreateCallback(Args &&... args);
	public:
		long long mOperatorId;
		std::string mMessageBuffer;
		std::string mBindSessionAdress;
		NetWorkManager * mNetWorkManager;
		LocalActionManager * mFunctionManager;
	};

	template<typename T>
	inline bool RemoteScheduler::Call(std::string func, NetWorkRetAction2<T> action)
	{
		auto pAction = make_shared<NetWorkRetActionBox2<T>>(action, func);
		return this->SendCallMessage(func, nullptr, pAction);
	}

	template<typename T>
	inline bool RemoteScheduler::Call(std::string func, google::protobuf::Message * message, NetWorkRetAction2<T> action)
	{
		auto pAction = make_shared<NetWorkRetActionBox2<T>>(action, func);
		return this->SendCallMessage(func, message, pAction);
	}
	template<typename T, typename ...Args>
	inline unsigned long long RemoteScheduler::CreateCallback(Args && ...args)
	{
		T * pAction = new T(std::forward<Args>(args)...);
		return this->mFunctionManager->AddCallback(pAction);
	}
}
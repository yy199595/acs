
#include<CommonXCode/XCode.h>
#include<CommonProtocol/Common.pb.h>
#include<CommonNetWork/TcpClientSession.h>
#include<CommonNetWork/NetWorkRetAction.h>
#include<CommonManager/ActionManager.h>
#include<CommonManager/NetWorkManager.h>
#include<CommonScript/LuaType/LuaTable.h>
namespace SoEasy
{
	class RemoteScheduler
	{
	public:
		RemoteScheduler(shared_ptr<TcpClientSession>, long long operaotrId = 0);
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
		bool SendCallMessage(std::string & func, LuaTable & message, NetWorkRetActionBox * action = nullptr);
		bool SendCallMessage(std::string & func, Message * message = nullptr, NetWorkRetActionBox * action = nullptr);		
	public:
		template<typename T, typename ... Args>
		unsigned long long CreateCallback(Args &&... args);
	public:
		long long mOperatorId;
		std::string mMessageBuffer;
		std::string mBindSessionAdress;
		NetWorkManager * mNetWorkManager;
		ActionManager * mFunctionManager;
	};

	template<typename T>
	inline bool RemoteScheduler::Call(std::string func, NetWorkRetAction2<T> action)
	{
		NetWorkRetActionBox * pAction = new NetWorkRetActionBox2<T>(action, func);
		return this->SendCallMessage(func, nullptr, pAction);
	}

	template<typename T>
	inline bool RemoteScheduler::Call(std::string func, google::protobuf::Message * message, NetWorkRetAction2<T> action)
	{
		NetWorkRetActionBox * pAction = new NetWorkRetActionBox2<T>(action, func);
		return this->SendCallMessage(func, message, pAction);
	}
	template<typename T, typename ...Args>
	inline unsigned long long RemoteScheduler::CreateCallback(Args && ...args)
	{
		T * pAction = new T(std::forward<Args>(args)...);
		return this->mFunctionManager->AddCallback(pAction);
	}
}
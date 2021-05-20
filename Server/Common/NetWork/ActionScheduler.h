#pragma once
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	class NetWorkWaitCorAction;
	class LocalRetActionProxy;
	class CoroutineManager;
	class ActionManager;
	class NetWorkManager;

	class ActionScheduler
	{
	public:
		ActionScheduler(long long operId = 0);
		ActionScheduler(shared_ptr<TcpClientSession> session, long long operId = 0);
	public:
		XCode Call(const std::string & service, const std::string func, Message & returnData);
		XCode Call(const std::string & service, const std::string func, shared_ptr<Message> message = nullptr);
		XCode Call(const std::string & service, const std::string func, shared_ptr<Message> message, Message & returnData);
	private:
		void InitScheduler();
		XCode SendCallMessage(const std::string & service, const std::string & func, shared_ptr<Message> message, shared_ptr<LocalRetActionProxy> callBack);
	private:
		long long mOperatorId;
		std::string mSessionAddress;
		NetWorkManager * mNetWorkManager;
		ActionManager * mActionManager;
		CoroutineManager * mCoroutineScheduler;
	};
}
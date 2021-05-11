#pragma once
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	class NetWorkWaitCorAction;
	class LocalRetActionProxy;
	class CoroutineManager;
	class LocalActionManager;
	class NetWorkManager;

	class ActionScheduler
	{
	public:
		ActionScheduler(long long operId = 0);
		ActionScheduler(shared_ptr<TcpClientSession> session);
	public:
		XCode Call(const std::string func, Message & returnData);
		XCode Call(const std::string func, const Message * message = nullptr);
		XCode Call(const std::string func, const Message * message, Message & returnData);
	private:
		void InitScheduler();
		XCode SendCallMessage(const std::string & func, const Message * message, shared_ptr<LocalRetActionProxy> callBack);
	private:
		long long mOperatorId;
		std::string mSessionAddress;
		NetWorkManager * mNetWorkManager;
		LocalActionManager * mActionManager;
		CoroutineManager * mCoroutineScheduler;
	};
}
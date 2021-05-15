#pragma once
#include<XCode/XCode.h>
#include"NetLuaRetAction.h"
#include<Util/NumberBuilder.h>
#include<Protocol/Common.pb.h>
#include<NetWork/TcpClientSession.h>

using namespace PB;
namespace SoEasy
{

	using NetWorkRetAction1 = std::function<void(shared_ptr<TcpClientSession>,XCode)>;

	template<typename T>
	using NetWorkRetAction2 = std::function<void(shared_ptr<TcpClientSession>, XCode, const T &)>;

	class LocalRetActionProxy
	{
	public:
		LocalRetActionProxy(const std::string & name);
		virtual ~LocalRetActionProxy() { }
	public:
		long long GetCreateTime() { return this->mCreateTime; }
		std::string & GetFunctionName() { return this->mFunctionName; }
	public:
		virtual void Invoke(shared_ptr<TcpClientSession>, shared_ptr<NetWorkPacket> backData) = 0;
	private:
		long long mActionKey;
		long long mCreateTime;
		std::string mFunctionName;
	protected:
		
	};

}

namespace SoEasy
{
	class LocalRetActionProxy1 : public LocalRetActionProxy
	{
	public:
		LocalRetActionProxy1(NetWorkRetAction1 action, std::string & name)
			: LocalRetActionProxy(name), mBindAction(action) {}

		~LocalRetActionProxy1() { }

	public:
		void Invoke(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> backData) override;
	private:
		NetWorkRetAction1 mBindAction;
	};

	template<typename T>
	class LocalRetActionProxy2 : public LocalRetActionProxy
	{
	public:
		LocalRetActionProxy2(NetWorkRetAction2<T> action, std::string & name)
			:LocalRetActionProxy(name), mBindAction(action) { }

		~LocalRetActionProxy2() { }

	public:
		void Invoke(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> backData) override
		{
			mReturnData.Clear();
			XCode code = (XCode)backData->error_code();
			if (code != XCode::TimeoutAutoCall)
			{
				const std::string & message = backData->message_data();			
				if (!mReturnData.ParseFromArray(message.c_str(), message.size()))
				{
					this->mBindAction(session, XCode::ParseMessageError, mReturnData);
					SayNoDebugError("parse " << typeid(T).name() << " error code:" << code);
					return;
				}
			}
			this->mBindAction(session, code, mReturnData);
		}
	private:
		T mReturnData;
		NetWorkRetAction2<T> mBindAction;
	};


	class LocalLuaRetActionProxy : public LocalRetActionProxy
	{
	public:
		LocalLuaRetActionProxy(NetLuaRetAction * action, std::string name);

		~LocalLuaRetActionProxy() { if (mBindLuaAction) { delete mBindLuaAction; } }

	public:
		void Invoke(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> backData);
	private:
		NetLuaRetAction * mBindLuaAction;
	};

	class LocalWaitRetActionProxy : public LocalRetActionProxy
	{
	public:
		LocalWaitRetActionProxy(NetLuaWaitAction * action, std::string name);

		~LocalWaitRetActionProxy() { if (mBindLuaAction) { delete mBindLuaAction; } }

	public:
		void Invoke(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> backData) override;
	private:
		NetLuaWaitAction * mBindLuaAction;
	};

	class CoroutineManager;

	class NetWorkWaitCorAction : public LocalRetActionProxy
	{
	public:
		NetWorkWaitCorAction(std::string name, CoroutineManager *);
		~NetWorkWaitCorAction() { std::cout << "delete NetWorkWaitCorAction" << std::endl; }
		static shared_ptr<NetWorkWaitCorAction> Create(std::string name, CoroutineManager *);
	public:
		void Invoke(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> backData) override;
	public:
		XCode GetCode() { return this->mCode; }
		const std::string & GetMsgData() { return this->mMessage; }
	private:
		XCode mCode;
		std::string mMessage;
		long long mCoroutineId;
		CoroutineManager * mScheduler;
	};
}
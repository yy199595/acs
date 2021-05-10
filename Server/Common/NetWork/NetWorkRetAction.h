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
		LocalRetActionProxy(std::string & name);
		virtual ~LocalRetActionProxy() { }
	public:
		long long GetCreateTime() { return this->mCreateTime; }
		std::string & GetFunctionName() { return this->mFunctionName; }
	public:
		virtual void Invoke(shared_ptr<TcpClientSession>, const shared_ptr<NetWorkPacket> backData) = 0;
	private:
		long long mActionKey;
		long long mCreateTime;
		std::string mFunctionName;
	protected:
		
	};

}

namespace SoEasy
{
	class NetWorkRetActionBox1 : public LocalRetActionProxy
	{
	public:
		NetWorkRetActionBox1(NetWorkRetAction1 action, std::string & name)
			: LocalRetActionProxy(name), mBindAction(action),mBindLuaAction(nullptr) {}

		NetWorkRetActionBox1(NetLuaRetAction * action, std::string & name)
			:LocalRetActionProxy(name), mBindAction(nullptr), mBindLuaAction(action) { }

		~NetWorkRetActionBox1() { if (mBindLuaAction) { delete mBindLuaAction; } }

	public:
		void Invoke(shared_ptr<TcpClientSession> session, const shared_ptr<NetWorkPacket> backData) override;
	private:
		NetWorkRetAction1 mBindAction;
		NetLuaRetAction * mBindLuaAction;
	};

	template<typename T>
	class NetWorkRetActionBox2 : public LocalRetActionProxy
	{
	public:
		NetWorkRetActionBox2(NetWorkRetAction2<T> action, std::string & name)
			:LocalRetActionProxy(name), mBindAction(action) { }

		~NetWorkRetActionBox2() { }

	public:
		void Invoke(shared_ptr<TcpClientSession> session, const shared_ptr<NetWorkPacket> backData) override
		{
			mReturnData.Clear();
			XCode code = (XCode)backData.error_code();
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


	class NetWorkRetActionBoxLua : public LocalRetActionProxy
	{
	public:
		NetWorkRetActionBoxLua(NetLuaRetAction * action, std::string name);

		~NetWorkRetActionBoxLua() { if (mBindLuaAction) { delete mBindLuaAction; } }

	public:
		void Invoke(shared_ptr<TcpClientSession> session, const shared_ptr<NetWorkPacket> backData);
	private:
		NetLuaRetAction * mBindLuaAction;
	};

	class NetWorkWaitActionBoxLua : public LocalRetActionProxy
	{
	public:
		NetWorkWaitActionBoxLua(NetLuaWaitAction * action, std::string name);

		~NetWorkWaitActionBoxLua() { if (mBindLuaAction) { delete mBindLuaAction; } }

	public:
		void Invoke(shared_ptr<TcpClientSession> session, const shared_ptr<NetWorkPacket> backData) override;
	private:
		NetLuaWaitAction * mBindLuaAction;
	};

	class CoroutineManager;

	class NetWorkWaitCorAction : public LocalRetActionProxy
	{
	public:
		NetWorkWaitCorAction(std::string name, CoroutineManager *);
	public:
		void Invoke(shared_ptr<TcpClientSession> session, const shared_ptr<NetWorkPacket> backData) override;
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
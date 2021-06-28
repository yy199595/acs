#pragma once
#include <XCode/XCode.h>
#include "TcpClientSession.h"
#include <Protocol/com.pb.h>
#include <Other/ObjectFactory.h>
#include<Pool/ObjectPool.h>
using namespace PB;
namespace SoEasy
{

	using RegisterAction = std::function<XCode(const std::string &, shared_ptr<NetWorkPacket>)>;

	using LocalAction1 = std::function<XCode(long long)>;

	template <typename T>
	using LocalAction2 = std::function<XCode(long long, const T &)>;

	template <typename T1, typename T2>
	using LocalAction3 = std::function<XCode(long long, const T1 &, T2 &)>;

	template <typename T>
	using LocalAction4 = std::function<XCode(long long, T &)>;

	using MysqlOperAction = std::function<XCode(Message &)>;
	using MysqlQueryAction = std::function<XCode(Message &, Message &)>;

	class LocalActionProxy
	{
	public:
		LocalActionProxy(std::string &name) { this->mActionName = name; }
		virtual ~LocalActionProxy() {}

	public:
		const std::string &GetName() { return this->mActionName; }
		virtual XCode Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket>) = 0;

	protected:
		std::string mActionName;
	};
}

namespace SoEasy
{
	class LocalActionProxy1 : public LocalActionProxy // 无参数 无返回
	{
	public:
		LocalActionProxy1(LocalAction1 action, std::string &name)
			: LocalActionProxy(name), mBindAction(action) {}

	public:
		XCode Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData) final;

	private:
		LocalAction1 mBindAction;
	};
}

namespace SoEasy
{
	class LocalMysqlActionProxy : public LocalActionProxy
	{
	public:
		LocalMysqlActionProxy(MysqlOperAction action, std::string &name)
			: LocalActionProxy(name), mBindAction(action) {}

	public:
		XCode Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData) final;

	private:
		MysqlOperAction mBindAction;
	};
}

namespace SoEasy
{
	class LocalMysqlQueryActionProxy : public LocalActionProxy
	{
	public:
		LocalMysqlQueryActionProxy(MysqlQueryAction action, std::string &name)
			: LocalActionProxy(name), mBindAction(action) {}

	public:
		XCode Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData) final;
	private:
		MysqlQueryAction mBindAction;
	};
}

namespace SoEasy
{
	template <typename T1>
	class LocalActionProxy2 : public LocalActionProxy //有参数 无返回
	{
	public:
		LocalActionProxy2(LocalAction2<T1> action, std::string &name) : LocalActionProxy(name), mBindAction(action) {}

	public:
		XCode Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData) final;

	private:
		std::string mMessageBuffer;
		LocalAction2<T1> mBindAction;
		ObjectPool<T1> mRequestPool;
	};

	template <typename T1>
	inline XCode LocalActionProxy2<T1>::Invoke(const shared_ptr<NetWorkPacket> requestMessage, shared_ptr<NetWorkPacket> responseMessage)
	{
		const long long operId = requestMessage->entityid();
		const long long callbackId = requestMessage->rpcid();
		const std::string &message = requestMessage->messagedata();

		T1 * mRequestData = this->mRequestPool.Create();
		if (!mRequestData->ParseFromArray(message.c_str(), message.size()))
		{
			this->mRequestPool.Destory(mRequestData);
			SayNoDebugError("parse proto fail : " << mRequestData->GetTypeName());
			return XCode::ParseMessageError;
		}
		XCode code = this->mBindAction(operId, *mRequestData);
		this->mRequestPool.Destory(mRequestData);
		return code;
	}
}

namespace SoEasy
{
	template <typename T1, typename T2>
	class LocalActionProxy3 : public LocalActionProxy //一个参数 一个返回
	{
	public:
		LocalActionProxy3(LocalAction3<T1, T2> action, std::string &name)
			: LocalActionProxy(name), mBindAction(action) {}

	public:
		XCode Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData) override;

	private:
		std::string mMessageBuffer;
		ObjectPool<T1> mRequestDataPool;
		ObjectPool<T2> mResponseDataPool;
		LocalAction3<T1, T2> mBindAction;
	};
	template <typename T1, typename T2>
	inline XCode LocalActionProxy3<T1, T2>::Invoke(const shared_ptr<NetWorkPacket> requestMessage, shared_ptr<NetWorkPacket> responseMessage)
	{
		T1 * requestData = this->mRequestDataPool.Create();
		const long long operId = requestMessage->entityid();
		const std::string &message = requestMessage->messagedata();
		if (!requestData->ParseFromArray(message.c_str(), message.size()))
		{
			this->mRequestDataPool.Destory(requestData);
			SayNoDebugError("parse proto fail : " << requestData->GetTypeName());
			return XCode::ParseMessageError;
		}
		T2 * responseData = this->mResponseDataPool.Create();
		XCode code = this->mBindAction(operId, *requestData, *responseData);
		if (responseData->SerializePartialToString(&mMessageBuffer))
		{
			responseMessage->set_messagedata(mMessageBuffer);
			responseMessage->set_protocname(responseData->GetTypeName());
		}
		this->mRequestDataPool.Destory(requestData);
		this->mResponseDataPool.Destory(responseData);
		return code;
	}
}

namespace SoEasy
{
	template<typename T1>
	class LocalActionProxy4 : public LocalActionProxy //无参数 一个返回
	{
	public:
		LocalActionProxy4(LocalAction4<T1> action, std::string & name) :LocalActionProxy(name), mBindAction(action) { }
		XCode Invoke(shared_ptr<TcpClientSession>session, const shared_ptr<NetWorkPacket> requestMessage, shared_ptr<NetWorkPacket> responseMessage) override
		{
			T1 * responseData = mResponseDataPool.Create();
			const long long operId = requestMessage->entityid();
			XCode code = this->mBindAction(session, operId, *responseData);
			if (responseData->SerializePartialToString(&mMessageBuffer))
			{
				responseMessage->set_messagedata(mMessageBuffer);
				responseMessage->set_protocname(responseData->GetTypeName());
			}
			this->mResponseDataPool.Destory(responseData);
			return code;
		}
	private:
		std::string mMessageBuffer;
		LocalAction4<T1> mBindAction;
		ObjectPool<T1> mResponseDataPool;
	};
}

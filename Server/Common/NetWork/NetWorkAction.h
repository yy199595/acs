#pragma once
#include <XCode/XCode.h>
#include "TcpClientSession.h"
#include <Protocol/com.pb.h>
#include <Other/ObjectFactory.h>

using namespace PB;
namespace SoEasy
{

	using RegisterAction = std::function<XCode(const std::string &, shared_ptr<NetWorkPacket>)>;

	using LocalAction1 = std::function<XCode(long long)>;

	template <typename T>
	using LocalAction2 = std::function<XCode(long long, shared_ptr<T>)>;

	template <typename T1, typename T2>
	using LocalAction3 = std::function<XCode(long long, shared_ptr<T1>, shared_ptr<T2>)>;

	using MysqlOperAction = std::function<XCode(shared_ptr<Message>)>;
	using MysqlQueryAction = std::function<XCode(shared_ptr<Message>, shared_ptr<Message>)>;

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
	class LocalActionProxy1 : public LocalActionProxy
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
	class LocalActionProxy2 : public LocalActionProxy
	{
	public:
		LocalActionProxy2(LocalAction2<T1> action, std::string &name) : LocalActionProxy(name), mBindAction(action) {}

	public:
		XCode Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData) final;

	private:
		std::string mMessageBuffer;
		shared_ptr<T1> mRequestData;
		LocalAction2<T1> mBindAction;
	};

	template <typename T1>
	inline XCode LocalActionProxy2<T1>::Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		mRequestData = std::make_shared<T1>();
		const long long operId = requestData->entityid();
		const std::string &name = requestData->protocname();
		const long long callbackId = requestData->rpcid();
		const std::string &message = requestData->messagedata();
		if (!message.empty() && !name.empty())
		{
			if (!mRequestData->ParseFromArray(message.c_str(), message.size()))
			{
				SayNoDebugError("parse proto fail : " << name);
				return XCode::ParseMessageError;
			}
			return this->mBindAction(operId, mRequestData);
		}
		return XCode::Failure;
	}
}

namespace SoEasy
{
	template <typename T1, typename T2>
	class LocalActionProxy3 : public LocalActionProxy
	{
	public:
		LocalActionProxy3(LocalAction3<T1, T2> action, std::string &name)
			: LocalActionProxy(name), mBindAction(action) {}

	public:
		XCode Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData) override;

	private:
		std::string mMessageBuffer;
		shared_ptr<T2> mReturnData;
		shared_ptr<T1> mRequestData;
		LocalAction3<T1, T2> mBindAction;
	};
	template <typename T1, typename T2>
	inline XCode LocalActionProxy3<T1, T2>::Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		mRequestData = make_shared<T1>();
		const long long operId = requestData->entityid();
		const std::string &name = requestData->protocname();
		const std::string &message = requestData->messagedata();
		if (!mRequestData->ParseFromArray(message.c_str(), message.size()))
		{
			SayNoDebugError("parse proto fail : " << mRequestData->GetTypeName());
			return XCode::ParseMessageError;
		}
		mReturnData = make_shared<T2>();
		XCode code = this->mBindAction(operId, mRequestData, mReturnData);
		if (mReturnData->SerializePartialToString(&mMessageBuffer))
		{
			returnData->set_messagedata(mMessageBuffer);
			returnData->set_protocname(mReturnData->GetTypeName());
		}
		return code;
	}
}

//namespace SoEasy
//{
//	template<typename T1>
//	class NetWorkActionBox4 : public LocalActionProxy
//	{
//	public:
//		NetWorkActionBox4(NetWorkAction4<T1> action, std::string & name) :LocalActionProxy(name), mBindAction(action) { }
//		XCode Invoke(shared_ptr<TcpClientSession>session, const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData) override
//		{
//			mReturnData = make_shared<T1>();
//			const long long operId = requestData->operator_id();
//			XCode code = this->mBindAction(session, operId, mReturnData);
//			if (mReturnData->SerializePartialToString(&mMessageBuffer))
//			{
//				returnData->set_message_data(mMessageBuffer);
//				returnData->set_protoc_name(mReturnData->GetTypeName());
//			}
//			return code;
//		}
//	private:
//		std::string mMessageBuffer;
//		shared_ptr<T1> mReturnData;
//		NetWorkAction4<T1> mBindAction;
//	};
//}

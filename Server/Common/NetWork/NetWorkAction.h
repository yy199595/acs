#pragma once
#include<XCode/XCode.h>
#include"TcpClientSession.h"
#include<Protocol/com.pb.h>
#include<Other/ObjectFactory.h>
#include<Pool/ObjectPool.h>
#ifdef SOEASY_DEBUG
#include<google/protobuf/util/json_util.h>
#endif // SOEASY_DEBUG

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

	class LocalActionProxy
	{
	public:
		LocalActionProxy(std::string &name) { this->mActionName = name; }
		virtual ~LocalActionProxy() {}
	public:
		const std::string &GetName() { return this->mActionName; }
		virtual XCode Invoke(PB::NetWorkPacket * messageData) = 0;
	public:
		const std::string & GetRequestMsgName() { return this->mRequestMsgName; }
		const std::string & GetResponseMsgName() { return this->mResponseMsgName; }
	protected:
		std::string mActionName;
		std::string mRequestMsgName;
		std::string mResponseMsgName;
	public:
#ifdef SOEASY_DEBUG
		std::string mServiceName;
#endif

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
		XCode Invoke(PB::NetWorkPacket * messageData) final;

	private:
		LocalAction1 mBindAction;
	};
}

namespace SoEasy
{
	template <typename T1>
	class LocalActionProxy2 : public LocalActionProxy //有参数 无返回
	{
	public:
		LocalActionProxy2(LocalAction2<T1> action, std::string &name) : LocalActionProxy(name), mBindAction(action) 
		{
			T1 * message = mRequestPool.Create();
			this->mRequestMsgName = message->GetTypeName();
			mRequestPool.Destory(message);
		}

	public:
		XCode Invoke(PB::NetWorkPacket * messageData) final;

	private:
		std::string mMessageBuffer;
		LocalAction2<T1> mBindAction;
		ObjectPool<T1> mRequestPool;
	};

	template <typename T1>
	inline XCode LocalActionProxy2<T1>::Invoke(PB::NetWorkPacket * messageData)
	{
		const long long operId = messageData->entityid();
		const long long callbackId = messageData->rpcid();
		const std::string &message = messageData->messagedata();

		T1 * mRequestData = this->mRequestPool.Create();
		if (!mRequestData->ParseFromArray(message.c_str(), message.size()))
		{
			this->mRequestPool.Destory(mRequestData);
			SayNoDebugError("parse proto fail : " << mRequestData->GetTypeName());
			return XCode::ParseMessageError;
		}
#ifdef SOEASY_DEBUG
		std::string json;
		util::MessageToJsonString(*mRequestData, &json);
		SayNoDebugWarning("[request ] [" << this->mServiceName << "." << this->GetName()
			<< "(" << mRequestData->GetTypeName() << ")] : " << json);
#endif
		XCode code = this->mBindAction(operId, *mRequestData);
#ifdef SOEASY_DEBUG
		if (messageData->rpcid() != 0)
		{
			SayNoDebugWarning("[response] [" << this->mServiceName << "." << this->GetName() << "]");
		}		
#endif
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
			: LocalActionProxy(name), mBindAction(action) 
		{
			T1 * message = mRequestDataPool.Create();
			this->mRequestMsgName = message->GetTypeName();
			mRequestDataPool.Destory(message);

			T2 * message2 = mResponseDataPool.Create();
			this->mResponseMsgName = message2->GetTypeName();
			mResponseDataPool.Destory(message2);
		}

	public:
		XCode Invoke(PB::NetWorkPacket * messageData) override;

	private:
		std::string mMessageBuffer;
		ObjectPool<T1> mRequestDataPool;
		ObjectPool<T2> mResponseDataPool;
		LocalAction3<T1, T2> mBindAction;
	};
	template <typename T1, typename T2>
	inline XCode LocalActionProxy3<T1, T2>::Invoke(PB::NetWorkPacket * messageData)
	{
		T1 * requestData = this->mRequestDataPool.Create();
		const long long operId = messageData->entityid();
		const std::string &message = messageData->messagedata();
		if (!requestData->ParseFromArray(message.c_str(), message.size()))
		{
			this->mRequestDataPool.Destory(requestData);
			SayNoDebugError("parse proto fail : " << requestData->GetTypeName());
			return XCode::ParseMessageError;
		}
		T2 * responseData = this->mResponseDataPool.Create();
#ifdef SOEASY_DEBUG
		std::string json;
		util::MessageToJsonString(*requestData, &json);
		SayNoDebugWarning("[request ] [" << this->mServiceName << "." << this->GetName()
			<< "(" << requestData->GetTypeName() << ")] : " << json);
#endif
		XCode code = this->mBindAction(operId, *requestData, *responseData);
		if (responseData->SerializePartialToString(&mMessageBuffer))
		{
			messageData->set_messagedata(mMessageBuffer);
			messageData->set_protocname(responseData->GetTypeName());
		}
#ifdef SOEASY_DEBUG
		util::MessageToJsonString(*responseData, &json);
		SayNoDebugWarning("[response] [" << this->mServiceName << "." << this->GetName()
			<<"(" << responseData->GetTypeName() << ")] : " << json);
#endif
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
		LocalActionProxy4(LocalAction4<T1> action, std::string & name) :LocalActionProxy(name), mBindAction(action)
		{
			T1 * message = mResponseDataPool.Create();
			this->mResponseMsgName = message->GetTypeName();
			this->mResponseDataPool.Destory(message);
		}

		XCode Invoke(PB::NetWorkPacket * messageData) override
		{
			T1 * responseData = mResponseDataPool.Create();
			const long long operId = messageData->entityid();
#ifdef SOEASY_DEBUG
			SayNoDebugWarning("[request ] [" << this->mServiceName << "." << this->GetName() << "]");
#endif
			XCode code = this->mBindAction(operId, *responseData);
#ifdef SOEASY_DEBUG
			std::string json;
			util::MessageToJsonString(*responseData, &json);
			SayNoDebugWarning("[response] [" << this->mServiceName << "." << this->GetName()
				<< "(" << responseData->GetTypeName() << ")] : " << json);
#endif
			if (responseData->SerializePartialToString(&mMessageBuffer))
			{
				messageData->set_messagedata(mMessageBuffer);
				messageData->set_protocname(responseData->GetTypeName());
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

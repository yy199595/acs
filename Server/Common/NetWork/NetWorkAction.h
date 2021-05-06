#pragma once
#include<XCode/XCode.h>
#include"NetLuaAction.h"
#include"TcpClientSession.h"
#include<Protocol/Common.pb.h>
#include<Other/ObjectFactory.h>

using namespace PB;
namespace SoEasy
{

	using RegisterAction = std::function<XCode(const std::string &, NetWorkPacket &)>;

	using NetWorkAction1 = std::function<XCode(shared_ptr<TcpClientSession>, long long)>;

	template<typename T>
	using NetWorkAction2 = std::function<XCode(shared_ptr<TcpClientSession>, long long, const T &)>;

	template<typename T1, typename T2>
	using NetWorkAction3 = std::function<XCode(shared_ptr<TcpClientSession>, long long, const T1 &, T2 &)>;

	template<typename T>
	using NetWorkAction4 = std::function<XCode(shared_ptr<TcpClientSession>, long long, T &)>;

	class NetWorkActionBox
	{
	public:
		NetWorkActionBox(std::string & name):mLuaFunction(nullptr) { this->mActionName = name; }
		virtual ~NetWorkActionBox() { }
	public:
		void BindLuaFunction(NetLuaAction * func);
	public:
		const std::string & GetName() { return this->mActionName; }
		virtual XCode Invoke(shared_ptr<TcpClientSession>, const NetWorkPacket & requestData, NetWorkPacket &) = 0;
	protected:
		std::string mActionName;
		NetLuaAction * mLuaFunction;
	};
}

namespace SoEasy
{
	class NetWorkActionBox1 : public NetWorkActionBox
	{
	public:
		NetWorkActionBox1(NetWorkAction1 action, std::string & name) 
			: NetWorkActionBox(name), mBindAction(action) { }
	public:
		XCode Invoke(shared_ptr<TcpClientSession> session, const NetWorkPacket & requestData, NetWorkPacket & returnData) override;
	private:
		NetWorkAction1 mBindAction;
	};
}

namespace SoEasy
{
	template<typename T1>
	class NetWorkActionBox2 : public NetWorkActionBox
	{
	public:
		NetWorkActionBox2(NetWorkAction2<T1> action, std::string & name) :NetWorkActionBox(name), mBindAction(action) { }
		XCode Invoke(shared_ptr<TcpClientSession> session, const NetWorkPacket & requestData, NetWorkPacket & returnData) override;
	private:
		T1 mRequestData;
		NetWorkAction2<T1> mBindAction;

	};
	template<typename T1>
	inline XCode NetWorkActionBox2<T1>::Invoke(shared_ptr<TcpClientSession> session, const NetWorkPacket & requestData, NetWorkPacket & returnData)
	{
		mRequestData.Clear();
		const long long operId = requestData.operator_id();
		const std::string & name = requestData.protoc_name();
		const std::string & message = requestData.message_data();
		if (!mRequestData.ParseFromArray(message.c_str(), message.size()))
		{
			SayNoDebugError("parse proto fail : " << name);
			return XCode::ParseMessageError;
		}
		
		XCode code = XCode::Failure;
		if (this->mLuaFunction != nullptr)
		{
			return this->mLuaFunction->Inovke2<T1>(session, operId, mRequestData);
		}
		return this->mBindAction(session, operId, mRequestData);
	}
}


namespace SoEasy
{
	template<typename T1, typename T2>
	class NetWorkActionBox3 : public NetWorkActionBox
	{
	public:
		NetWorkActionBox3(NetWorkAction3<T1, T2> action, std::string & name) 
			:NetWorkActionBox(name), mBindAction(action) { }
	public:
		XCode Invoke(shared_ptr<TcpClientSession>session, const NetWorkPacket & requestData, NetWorkPacket & returnData) override;
	private:
		T2 mReturnData;
		T1 mRequestData;
		std::string mMessageBuffer;
		NetWorkAction3<T1, T2> mBindAction;
	};
	template<typename T1, typename T2>
	inline XCode NetWorkActionBox3<T1, T2>::Invoke(shared_ptr<TcpClientSession> session, const NetWorkPacket & requestData, NetWorkPacket & returnData)
	{
		mReturnData.Clear();
		mRequestData.Clear();
		const long long operId = requestData.operator_id();
		const std::string & name = requestData.protoc_name();
		const std::string & message = requestData.message_data();
		if (!mRequestData.ParseFromArray(message.c_str(), message.size()))
		{		
			SayNoDebugError("parse proto fail : " << mRequestData.GetTypeName());
			return XCode::ParseMessageError;
		}
		XCode code = XCode::Failure;
		if (this->mLuaFunction != nullptr)
		{
			code = this->mLuaFunction->Inovke3<T1, T2>(session, operId, mRequestData, mReturnData);
		}
		else
		{
			code = this->mBindAction(session, operId, mRequestData, mReturnData);
		}

		mMessageBuffer.clear();
		if (!mReturnData.SerializePartialToString(&mMessageBuffer))
		{
			SayNoDebugError("parse protobuf fail type : " << typeid(T2).name());
			return XCode::ParseMessageError;
		}
		returnData.set_message_data(mMessageBuffer);
		returnData.set_protoc_name(mReturnData.GetTypeName());
		return code;
	}
}

namespace SoEasy
{
	template<typename T1>
	class NetWorkActionBox4 : public NetWorkActionBox
	{
	public:
		NetWorkActionBox4(NetWorkAction4<T1> action, std::string & name) :NetWorkActionBox(name), mBindAction(action) { }
		XCode Invoke(shared_ptr<TcpClientSession>session, const NetWorkPacket & requestData, NetWorkPacket & returnData) override
		{					
			mReturnData.Clear();
			const long long operId = requestData.operator_id();
			XCode code = this->mLuaFunction == nullptr 
				? this->mBindAction(session, operId, mReturnData)
				: this->mLuaFunction->Inovke4(session, operId, mReturnData);

			if (!mReturnData.SerializePartialToString(&mMessageBuffer))
			{
				SayNoDebugError("parse protobuf fail type : " << typeid(T1).name());
				return XCode::ParseMessageError;
			}
			returnData.set_message_data(mMessageBuffer);
			returnData.set_protoc_name(mReturnData.GetTypeName());
			return code;
		}
	private:
		T1 mReturnData;
		std::string mMessageBuffer;
		NetWorkAction4<T1> mBindAction;
	};
}


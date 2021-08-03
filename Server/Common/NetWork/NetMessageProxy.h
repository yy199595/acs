#pragma once
#include <XCode/XCode.h>
#include <Other/ProtocolConfig.h>
#include <google/protobuf/message.h>
#include <Pool/ObjectPool.h>
using google::protobuf::Message;
namespace Sentry
{
	enum NetMessageType
	{
		s2sRequest = 1,
		c2sRequest = 2,
		c2sNotice = 3,
		s2cNotice = 4,
		s2sNotice = 5,

		RequestEnd = 100,

		s2sResponse = 101,
		c2sResponse = 103,

		ResponseEnd = 255
	};

	class NetMessageProxy
	{
	public:
		NetMessageProxy(NetMessageType type);
		~NetMessageProxy();

	public:
		static NetMessageProxy *Create(const char *message, const size_t size);
		static NetMessageProxy *Create(NetMessageType type, const std::string & service, const std::string &method);

	public:
		bool InitMessageParame(Message * message = nullptr, long long rcpId = 0, long long userId = 0);
	public:
		void Clear();
		size_t WriteToBuffer(char *buffer, const size_t size);
	public:
		const std::string &GetMsgBody() { return this->mMessageData; }
		const std::string &GetMethd() { return this->mProConfig->MethodName; }
		const std::string &GetService() { return this->mProConfig->ServiceName; }	
		const ProtocolConfig * GetProConfig() { return this->mProConfig; }
	public:
		bool InitMessageData(XCode code, Message * message); // response
		bool InitMessageData(Message * message, long long rpcId, long long userId); //request
		

	public:
		XCode GetCode() { return this->mCode; }
		long long GetRpcId() { return this->mRpcId; }
		long long GetUserId() { return this->mUserId; }
		NetMessageType GetMessageType() { return this->mMsgType; }	
	private:
		NetMessageType mMsgType;
	private:
		XCode mCode;
		long long mRpcId;
		long long mUserId;
	private:
		std::string mMessageData;
		const ProtocolConfig * mProConfig;
	};
}
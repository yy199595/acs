#pragma once
#include <XCode/XCode.h>
#include <Pool/ObjectPool.h>
#include <Other/ProtocolConfig.h>
#include <google/protobuf/message.h>
using google::protobuf::Message;
namespace Sentry
{
	enum NetMessageType
	{
		S2S_REQUEST = 1,
		C2S_REQUEST = 2,
		C2S_NOTICE = 3,
		S2C_REQUEST = 4,
		S2S_NOTICE = 5,

		REQUEST_END = 100,

		S2S_RESPONSE = 101,
		C2S_RESPONSE = 103,

		RESPONSE_END = 255
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
		void Clear();
		size_t WriteToBuffer(char *buffer, const size_t size);
	public:
		const std::string &GetMsgBody() { return this->mMessageData; }
		const std::string &GetMethd() { return this->mProConfig->MethodName; }
		const std::string &GetService() { return this->mProConfig->ServiceName; }	
		const ProtocolConfig * GetProConfig() { return this->mProConfig; }
	public:
		void SetType(NetMessageType type);
		void SetCode(XCode code) { this->mCode = code; }
		void SetRpcId(const long long id) { this->mRpcId = id; }
		void SetUserId(const long long id) { this->mUserId = id; }
		void ClearMessage() { this->mMessageData.clear(); }
		bool SetMessage(const Message & message) { return message.SerializePartialToString(&mMessageData); }
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
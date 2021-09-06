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
		S2C_NOTICE = 104,
		RESPONSE_END = 255
	};

	class PacketMapper
	{
	public:
		PacketMapper(NetMessageType type);
		~PacketMapper();

	public:
		static void Destory(PacketMapper * message);
		static PacketMapper *Create(const std::string & address, const char *message, const size_t size);
		static PacketMapper *Create(const std::string & address, NetMessageType type, const std::string & service, const std::string &method);
	public:
		void Clear();
		size_t WriteToBuffer(char *buffer, const size_t size);
	public:
		const std::string &GetAddress() { return this->mAddress; }
		const std::string &GetMsgBody() { return this->mMessageData; }
		const std::string &GetMethd() { return this->mProConfig->MethodName; }
		const std::string &GetService() { return this->mProConfig->ServiceName; }
		const ProtocolConfig * GetProConfig() { return this->mProConfig; }
	public:
		bool SetCode(XCode code);
		void SetType(NetMessageType type);
		void SetUserId(const long long id) { this->mUserId = id; }
		bool SetRpcId(const unsigned int id);
		void ClearMessage() { this->mMessageData.clear(); }
		bool SetMessage(const Message * message);
		bool SetMessage(const Message & message);
	public:
		XCode GetCode() { return this->mCode; }
		long long GetUserId() { return this->mUserId; }
		unsigned int GetRpcId() { return this->mRpcId; }
		NetMessageType GetMessageType() { return this->mMsgType; }
		size_t GetPackageSize();
	private:
		XCode mCode;
		long long mUserId;
		unsigned int mRpcId;
		NetMessageType mMsgType;
	private:
		std::string mAddress;
		std::string mMessageData;
		const ProtocolConfig * mProConfig;
		std::queue<PacketMapper *> mNetPool;
		std::queue<PacketMapper *> mMainPool;
	};
}
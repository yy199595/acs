//
// Created by zmhy0073 on 2022/9/27.
//

#ifndef APP_MESSAGE_H
#define APP_MESSAGE_H
#include"Rpc.h"
#include<string>
#include<unordered_map>
#include"Proto/Include/Message.h"
#include"Proto/Message/IProto.h"
#include"Yyjson/Document/Document.h"

#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif
namespace rpc
{
	class Head : public tcp::IProto, public tcp::IHeader
	{
	public:
		bool GetKeys(std::vector<std::string>& keys) const;
		const std::string& GetStr(const std::string& key) const;
	public:
		size_t GetLength() const;
		void Clear() final{ this->mHeader.clear();}
	public:
		int OnSendMessage(std::ostream& os) final;
		int OnRecvMessage(std::istream& os, size_t size) final;
	};

	class Message : public tcp::IProto
#ifdef __SHARE_PTR_COUNTER__
	, public memory::Object<Message>
#endif
	{
	public:
		Message();
		~Message() final = default;
#ifdef __MEMORY_POOL_OPERATOR__
	public:
		static void  operator delete(void * ptr);
		static void * operator new(std::size_t  size);
#endif
	public:
		int OnSendMessage(std::ostream& os) final;
		int OnRecvMessage(std::istream& os, size_t size) final;
	public:
		bool Decode(const char * message, int len);
	public:
		bool EncodeToJson(std::string & json);
		bool DecodeFromJson(json::r::Value & value);
		bool DecodeFromJson(const char * message, int len);
	public:
		int GetCode(int code = 1) const;
		std::unique_ptr<Message> Clone() const;
		inline Head& GetHead() { return this->mHead; }
		inline Head& TempHead(){ return this->mTempHead;}
		inline char GetNet() const{ return this->mNet;}
		inline const Head& ConstHead() const{ return this->mHead;}
		inline int GetRpcId() const{return this->mProtoHead.RpcId;}
		inline char GetType() const { return this->mProtoHead.Type;}
		inline char GetProto() const { return this->mProtoHead.Porto; }
		inline ProtoHead & GetProtoHead() { return this->mProtoHead;}
		inline const Head& ConstTempHead() const {return this->mTempHead;}
		inline int GetTimeout() const { return this->mTimeout; }
	public:
		void Init(const rpc::ProtoHead& protoHead);
		inline void SetSockId(int id){ this->mSockId = id;}
		inline void SetNet(char net){ this->mNet = net;}
		inline void SetRpcId(int id){ this->mProtoHead.RpcId = id; }
		inline void SetType(char type) { this->mProtoHead.Type = type; }
		inline void SetProto(char proto) { this->mProtoHead.Porto = proto; }
		inline void SetTimeout(int timeout) { this->mTimeout = timeout; }
	public:
		void Clear() final;
		std::string ToString() final;
		std::string* Body() { return &this->mBody; }
		inline int SockId() const { return this->mSockId; }
		inline size_t GetSize() const{ return this->mBody.size(); }
		inline const std::string& GetBody() const{ return this->mBody; }
		inline void Append(const std::string& data){ this->mBody.append(data); }
	public:
		void SetContent(const std::string& content);
		void SetContent(char proto, const std::string& content);
	public:
		bool ParseMessage(pb::Message* message);
		bool WriteMessage(const pb::Message* message);
	public:
		bool ParseMessage(json::r::Document* message);
		bool WriteMessage(json::w::Document* message);
	private:
		char mNet;
		Head mHead; //写入socket
		int mSockId;
		int mTimeout;
		Head mTempHead; //不写入socket
		std::string mBody;
		rpc::ProtoHead mProtoHead;
#ifdef __MEMORY_POOL_OPERATOR__
	private:
		static std::mutex sAllocLock;
		static std::vector<void *> sAllocQueue;
#endif
	};
}


#endif //APP_MESSAGE_H

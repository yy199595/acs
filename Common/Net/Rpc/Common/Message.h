//
// Created by zmhy0073 on 2022/9/27.
//

#ifndef APP_MESSAGE_H
#define APP_MESSAGE_H
#include"Net/Rpc/Common/Rpc.h"
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
	class Head final : public tcp::IProto, public tcp::IHeader
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

	class Message final : public tcp::IProto
#ifdef __SHARE_PTR_COUNTER__
	, public memory::Object<Message>
#endif
	{
	public:
		Message() noexcept;
		~Message() final = default;
#ifdef __MEMORY_POOL_OPERATOR__
	public:
		void  operator delete(void * ptr);
		void * operator new(std::size_t  size);
#endif
	public:
		int OnSendMessage(std::ostream& os) final;
		int OnRecvMessage(std::istream& os, size_t size) final;
	public:
		bool Decode(const char * message, int len);
	public:
		bool EncodeToJson(std::string & json);
		bool DecodeFromJson(json::r::Value & value);
		bool DecodeFromJson(const char * message, size_t len);
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
		inline char GetSource() const { return this->mProtoHead.Source; }
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
		inline void SetSource(char source) { this->mProtoHead.Source = source; }
		inline void SetTimeout(int timeout) { this->mTimeout = timeout; }
	public:
		void Clear() final;
		std::string ToString() final;
		std::string* Body() { return &this->mBody; }
		inline int SockId() const { return this->mSockId; }
		inline const std::string& GetBody() const{ return this->mBody; }
		inline void Append(const std::string& data){ this->mBody.append(data); }
		inline size_t CalcMessageLength() final { return this->mHead.GetLength() + this->mBody.size(); }
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



namespace rpc
{
	class IInnerSender //内网接口
	{
	public:
		IInnerSender() = default;
		virtual ~IInnerSender() = default;
	public:
		virtual void Remove(int id) { }
		virtual char GetNet() const noexcept = 0;
		virtual int Send(int id, rpc::Message* message) noexcept = 0;
		virtual int Connect(const std::string & address) { return 0; }
	};

	class IOuterSender //外网接口
	{
	public:
		virtual char GetNet() const noexcept = 0;
		virtual int Send(int id, rpc::Message* message) noexcept = 0;
		virtual void Broadcast(rpc::Message* message) noexcept = 0;
	};
}


namespace tcp
{
	namespace Data
	{
		inline void Read(const char * buffer, rpc::ProtoHead & head)
		{
			int offset = sizeof(head.Len);
			tcp::Data::Read(buffer, head.Len);
			{
				head.Type = buffer[offset++];
				head.Porto = buffer[offset++];
				head.Source = buffer[offset++];
			}
			tcp::Data::Read(buffer + offset, head.RpcId);
		}
		template<> inline void Read(std::istream& os, rpc::ProtoHead& value)
		{
			char buffer[rpc::RPC_PACK_HEAD_LEN] = { 0 };
			os.readsome(buffer, rpc::RPC_PACK_HEAD_LEN);

			int offset = sizeof(value.Len);
			tcp::Data::Read(buffer, value.Len);
			{
				value.Type = buffer[offset++];
				value.Porto = buffer[offset++];
				value.Source = buffer[offset++];
			}
			tcp::Data::Read(buffer + offset, value.RpcId);

		}

		template<> inline void Write(std::ostream& is, const rpc::ProtoHead & value)
		{
			char buffer[rpc::RPC_PACK_HEAD_LEN] = { 0 };

			int offset = sizeof(value.Len);
			tcp::Data::Write(buffer, value.Len);
			{
				buffer[offset++] = value.Type;
				buffer[offset++] = value.Porto;
				buffer[offset++] = value.Source;
			}
			tcp::Data::Write(buffer + offset, value.RpcId);
			is.write(buffer, rpc::RPC_PACK_HEAD_LEN);
		}
	}
}


#endif //APP_MESSAGE_H

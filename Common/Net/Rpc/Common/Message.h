//
// Created by zmhy0073 on 2022/9/27.
//

#ifndef APP_MESSAGE_H
#define APP_MESSAGE_H

#include"Net/Rpc/Common/Rpc.h"
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
	public:
		int OnSendMessage(std::ostream& os) final;
		int OnRecvMessage(std::istream& os, size_t size) final;
	public:
		bool IsOk();
		int GetCode(int code = 1) const;
		std::unique_ptr<Message> Clone() const;
		inline Head& GetHead() { return this->mHead; }
		inline Head& TempHead(){ return this->mTempHead;}
		inline char GetNet() const{ return this->mNet;}
		inline const Head& ConstHead() const{ return this->mHead;}
		inline int GetRpcId() const{return this->mProtoHead.rpcId;}
		inline char GetType() const { return this->mProtoHead.type;}
		inline char GetProto() const { return this->mProtoHead.porto; }
		inline char GetSource() const { return this->mProtoHead.source; }
		inline ProtoHead & GetProtoHead() { return this->mProtoHead;}
	public:
		void Init(const rpc::ProtoHead& protoHead);
		inline void SetMsg(char type) { this->mMsg = type; }
		inline void SetSockId(int id){ this->mSockId = id;}
		inline void SetNet(char net){ this->mNet = net;}
		inline void SetRpcId(int id){ this->mProtoHead.rpcId = id; }
		inline void SetType(char type) { this->mProtoHead.type = type; }
		inline void SetProto(char proto) { this->mProtoHead.porto = proto; }
		inline void SetSource(char source) { this->mProtoHead.source = source; }
	public:
		void Clear() final;
		std::string ToString() final;
		std::string* Body() { return &this->mBody; }
		inline int SockId() const { return this->mSockId; }
		inline const std::string& GetBody() const{ return this->mBody; }
		inline void Append(const std::string& data){ this->mBody.append(data); }
	public:
		void SetError(const std::string& content);
		void SetContent(const std::string& content);
		void SetContent(const json::w::Document & content);
		void SetContent(char proto, const std::string& content);
		void SetContent(char proto, const char* content, size_t count);
		template<typename T>
		inline std::enable_if_t<std::is_floating_point<T>::value || std::is_integral<T>::value, void> SetContent(T value)
		{
			this->mBody = std::to_string(value);
			this->mProtoHead.porto = rpc::proto::number;
		}
	private:
		char mNet;
		char mMsg; //协议格式
		Head mHead; //写入socket
		int mSockId;
		Head mTempHead; //不写入socket
		std::string mBody;
		rpc::ProtoHead mProtoHead;
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
		virtual int Connect(const std::string & address) { return 0; }
		virtual int Send(int id, std::unique_ptr<rpc::Message> & message) noexcept = 0;
	};

	class IOuterSender //外网接口
	{
	public:
		virtual char GetNet() const noexcept = 0;
		virtual int Send(int id, std::unique_ptr<rpc::Message> & message) noexcept = 0;
		virtual void Broadcast(std::unique_ptr<rpc::Message> & message) noexcept = 0;
	};

	class IOuterMessage
	{
	public:
		virtual int OnMessage(std::unique_ptr<rpc::Message> & message) noexcept = 0;
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
				head.type = buffer[offset++];
				head.porto = buffer[offset++];
				head.source = buffer[offset++];
			}
			tcp::Data::Read(buffer + offset, head.rpcId);
		}
		inline void ReadHead(std::istream& os, rpc::ProtoHead& value, bool hasLength)
		{
			int offset = 0;
			char buffer[rpc::RPC_PACK_HEAD_LEN] = { 0 };
			int readLen = hasLength ? rpc::RPC_PACK_HEAD_LEN
					: (rpc::RPC_PACK_HEAD_LEN - rpc::RPC_PACKET_LEN_BYTES);

			os.readsome(buffer, readLen);
			if (hasLength)
			{
				offset = rpc::RPC_PACKET_LEN_BYTES;
				tcp::Data::Read(buffer, value.Len, rpc::RPC_PACKET_LEN_BYTES);
			}
			value.type = buffer[offset++];
			value.porto = buffer[offset++];
			value.source = buffer[offset++];
			tcp::Data::Read(buffer + offset, value.rpcId);
		}

		inline void WriteHead(std::ostream& is, const rpc::ProtoHead & value, bool hasLength)
		{
			int offset = 0;
			char buffer[rpc::RPC_PACK_HEAD_LEN] = { 0 };

			int writeLen = hasLength ? rpc::RPC_PACK_HEAD_LEN
									 : (rpc::RPC_PACK_HEAD_LEN - rpc::RPC_PACKET_LEN_BYTES);
			if (hasLength)
			{
				offset = rpc::RPC_PACKET_LEN_BYTES;
				tcp::Data::Write(buffer, value.Len, rpc::RPC_PACKET_LEN_BYTES);
			}

			buffer[offset++] = value.type;
			buffer[offset++] = value.porto;
			buffer[offset++] = value.source;
			tcp::Data::Write(buffer + offset, value.rpcId);
			is.write(buffer, writeLen);
		}
	}
}


#endif //APP_MESSAGE_H

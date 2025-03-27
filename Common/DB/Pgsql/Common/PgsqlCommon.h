//
// Created by 64658 on 2025/2/18.
//

#ifndef APP_PGSQLCOMMON_H
#define APP_PGSQLCOMMON_H
#include "DB/Common/Url.h"
#include "DB/Common/Explain.h"
#include "Proto/Message/IProto.h"

#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif
namespace pgsql
{
	constexpr unsigned int VERSION = 196608;
	struct Cluster : public json::Object<Cluster>
	{
	public:
		int ping = 15;
		int count = 1;
		int retry = 5;
		bool debug = false;
		db::Explain explain;
		int conn_count = 3;
		std::string script;
		std::vector<std::string> address;
	};

	struct Config : public db::Url
	{
	public:
		Config() : db::Url("pgsql") { }
	public:
		std::string db;
		std::string user;
		int conn_count = 3;
		std::string script;
		std::string address;
		std::string password;
	};

	namespace type
	{
		constexpr char status = 'S';
		constexpr char auth = 'R';
		constexpr char backend_key = 'K';
		constexpr char ready_for_query = 'Z';
		constexpr char query = 'Q';
		constexpr char notice = 'N';
		constexpr char notification = 'A';
		constexpr char password = 'p';
		constexpr char row_description = 'T';
		constexpr char data_row = 'D';
		constexpr char command_complete = 'C';
		constexpr char error = 'E';
	}

	namespace field
	{
		constexpr unsigned int BOOL = 16; //布尔类型，值为 true 或 false
		constexpr unsigned int NUMBER_INT16 = 21; //16 位有符号整数
		constexpr unsigned int NUMBER_INT32 = 23; //32 位有符号整数
		constexpr unsigned int NUMBER_INT64 = 20; //64 位有符号整数


		constexpr unsigned int NUMBER_FLOAT32 = 700; //单精度浮点数
		constexpr unsigned int NUMBER_FLOAT64 = 701; //双精度浮点数
		constexpr unsigned int NUMBER_NUMERIC = 1700; //任意精度的定点数

		constexpr unsigned int STRING_CHAR = 1042; //固定长度的字符串
		constexpr unsigned int STRING_VARCHAR = 1043; //可变长度的字符串，有长度限制
		constexpr unsigned int STRING_TEXT = 25; //可变长度的字符串，无长度限制
		constexpr unsigned int STRING_BYTES = 17; //二进制

		constexpr unsigned int STRING_JSON = 114; //json

	}

	namespace auth
	{
		constexpr unsigned int OK = 0;
		constexpr unsigned int PASSWORD = 3;  //明文密码
		constexpr unsigned int MD5_PASSWORD = 5; //md5密码
		constexpr unsigned int SCRAM_SHA_256 = 10; //sha256
	}

	struct FieldInfo
	{
		int type;
		std::string name;
	};
}

namespace pgsql
{
	inline unsigned int ReadLength(const char * buffer)
	{
		unsigned int length = (static_cast<unsigned char>(buffer[0]) << 24) |
							  (static_cast<unsigned char>(buffer[1]) << 16) |
							  (static_cast<unsigned char>(buffer[2]) << 8) |
							  static_cast<unsigned char>(buffer[3]);
		return length;
	}

	class Head
	{
	public:
		char type;
		unsigned int length;
	public:
		inline void Decode(const char * buffer)
		{
			this->type = buffer[0];
			this->length = ReadLength(buffer + 1);
		}
		inline void Decode(std::istream &os)
		{
			char buffer[5] = { 0};
			os.readsome(buffer, sizeof(buffer));
			this->Decode(buffer);
		}
	};

	class StartRequest : public tcp::IProto
	{
	public:
		StartRequest() = default;
	public:
		void Clear() final;
		int OnSendMessage(std::ostream &os) final;
	public:
		std::string db;
		std::string user;
	};
}

namespace pgsql
{
	class Request : public tcp::IProto
#ifdef __SHARE_PTR_COUNTER__
		, public memory::Object<Request>
#endif
	{
	public:
		explicit Request(std::string sql) :  mRpcId(0), mType(pgsql::type::query), mMessage(std::move(sql)) { }
		Request(unsigned char t, std::string message) : mRpcId(0), mType(t), mMessage(std::move(message)) { }
	public:
		void Clear() final;
		bool GetCommand(std::string & cmd) const;
		int OnSendMessage(std::ostream &os) final;
		void SetRpcId(int id) { this->mRpcId = id; }
		inline int GetRpcId() const { return this->mRpcId; }
		std::string ToString() final { return this->mMessage; }
	private:
		int mRpcId;
		unsigned char mType;
		std::string mMessage;
	};
}

namespace pgsql
{
	class Response
#ifdef __SHARE_PTR_COUNTER__
	: public memory::Object<Response>
#endif
	{
	public:
		int count = 0;
		std::string mCmd;
		std::string mError;
		std::vector<std::string> mResults;
	public:
		inline bool IsOk() const { return this->mError.empty(); }
	};
}

namespace pgsql
{
	class Result : public tcp::IProto
#ifdef __SHARE_PTR_COUNTER__
		, public memory::Object<Result>
#endif
	{
	public:
		Result();
	public:
		void Clear() final;
		int OnSendMessage(std::ostream &os) final;
		int OnRecvMessage(std::istream &os, size_t size) final;
	public:
		void DecodeData(Response & response);
	public:
		char mType;
		std::string mError;
		std::string mBuffer;
		std::string mResult;
		std::unordered_map<std::string, std::string> mStatus;
	private:
		int mDecodeStatus;
		std::vector<std::string> mRows;
		std::vector<pgsql::FieldInfo> mFields;
	};
}



#endif //APP_PGSQLCOMMON_H

//
// Created by yy on 2025/2/10.
//

#ifndef APP_MYSQLPROTO_H
#define APP_MYSQLPROTO_H

#include <vector>
#include"Proto/Message/IProto.h"
#include "Mysql/Common/MysqlCommon.h"

#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif

namespace mysql
{
	inline int ReadLength(const char * header)
	{
		return header[0] | (header[1] << 8) | (header[2] << 16);
	}

	template<typename T = uint8_t>
	inline T read(const char * buff, size_t size = sizeof(T))
	{
		T value = 0;
		std::memcpy(&value, buff, size);
		return value;
	}
	inline std::vector<uint8_t> encode(uint32_t length)
	{
		return std::vector<uint8_t>{ static_cast<uint8_t>(length & 0xFF),
				 static_cast<uint8_t>((length >> 8) & 0xFF),
				 static_cast<uint8_t>((length >> 16) & 0xFF) };
	}
}

namespace mysql
{
	class Request : public tcp::IProto
#ifdef __SHARE_PTR_COUNTER__
		, public memory::Object<Request>
#endif
	{
	public:
		explicit Request(char cmd);
		explicit Request(const std::string & message);
		explicit Request(const char * sql, size_t size);
		explicit Request(char cmd, std::string message);
	public:
		bool GetCommand(std::string & cmd) const;
		void SetRpcId(int id) { this->mRpcId = id; }
		int GetRpcId() const { return this->mRpcId; }
		void SetIndex(unsigned char idx) { this->mIndex = idx; }
		inline std::string ToString() final { return this->mMessage; }
	private:
		void Clear() final;
		int OnSendMessage(std::ostream& os) final;
	private:
		char mCmd;
		int mRpcId;
		std::string mMessage;
		unsigned char mIndex;
	};
}

namespace mysql
{
	class LoginRequest
	{
	public:
		std::string salt;
		std::string user;
		std::string database;
		std::string password;
		unsigned char charset;
		std::string authPlugin;
		bool enable_compression = false; // 新增：是否启用压缩
		int zstd_compression_level = 3;  // 新增：zstd 压缩级别，默认 3
	public:
		int Encode(std::vector<uint8_t> & packet) const;
	};
}

namespace mysql
{
	namespace plugin
	{
		constexpr const char * MYSQL_CLEAR_PASSWORD = "mysql_clear_password";
		constexpr const char * MYSQL_NATIVE_PASSWORD = "mysql_native_password";
		constexpr const char * CACHING_SHA2_PASSWORD = "caching_sha2_password";

		extern std::string native_password(const std::string & password, const std::string & salt);
#ifdef __ENABLE_OPEN_SSL__
		extern std::string caching_sha2_password(const std::string & password, const std::string & salt);
#endif
	}
}

namespace mysql
{
	class Response
#ifdef __SHARE_PTR_COUNTER__
	: public memory::Object<Request>
#endif
	{
	public:
		Response();
	public:
		void Clear() ;
		int OnRecvMessage(std::istream &os, size_t size);
	public:
		std::string ToString() const;
		unsigned char ReadChar(unsigned int &pos);
		std::string ReadString(unsigned int & pos);
		unsigned short ReadShort(unsigned int & pos);
		unsigned int DecodeColumnCount(unsigned int & pos);
		unsigned char GetIndex() const { return this->mIndex; }
		unsigned short GetErrorCode() const { return this->mErrorCode; }
		const std::string & GetBuffer() const { return this->mMessage;}
		unsigned char GetPackageCode() const { return this->mPackageCode; }
		bool IsOk() const { return this->mPackageCode != mysql::PACKAGE_ERR; }
		bool HasError() const { return this->mPackageCode == mysql::PACKAGE_ERR; }
		const mysql::OKResponse & GetOKResponse() const { return this->mOkResult; }
	public:
		bool GetFirstResult(std::string & result) const;
		mysql::Result & GetResult() { return this->mResult; }
	private:
		int OnMessage(const char * buffer, size_t size);
	private:
		int mLength;
		int mDecodeStatus;
		std::string mMessage;
		unsigned char mIndex;
		unsigned short mErrorCode;
		unsigned char mPackageCode;
	private:
		mysql::Result mResult;
		mysql::OKResponse mOkResult;
	};
}

#endif //APP_MYSQLPROTO_H

//
// Created by yy on 2025/2/10.
//

#ifndef APP_MYSQLPROTO_H
#define APP_MYSQLPROTO_H

#include <vector>
#include"Proto/Message/IProto.h"
#include "Mysql/Common/MysqlCommon.h"

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
	{
	public:
		explicit Request(std::string message);
		explicit Request(const char * sql, size_t size);
		Request(char cmd, std::string message);
	public:
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
	class LoginRequest : public tcp::IProto
	{
	public:
		LoginRequest();
	public:
		void Clear() final { };
		int OnSendMessage(std::ostream &os) final;
	public:
		std::string mSalt;
		std::string mUser;
		std::string mDatabase;
		std::string mPassword;
		unsigned char mCharset;
		std::string mAuthPlugin;
	};
}

namespace mysql
{
	class Response : public tcp::IProto
	{
	public:
		Response();
	public:
		void Clear() final;
		int OnSendMessage(std::ostream &os) final { return 0; }
		int OnRecvMessage(std::istream &os, size_t size) final;
	public:
		unsigned char ReadChar(unsigned int &pos);
		std::string ReadString(unsigned int & pos);
		unsigned short ReadShort(unsigned int & pos);
		unsigned int DecodeColumnCount(unsigned int & pos);
		mysql::Result & GetResult() { return this->mResult; }
		unsigned char GetIndex() const { return this->mIndex; }
		unsigned short GetErrorCode() const { return this->mErrorCode; }
		const std::string & GetBuffer() const { return this->mMessage;}
		unsigned char GetPackageCode() const { return this->mPackageCode; }
		bool IsOk() const { return this->mPackageCode != mysql::PACKAGE_ERR; }
		const mysql::OKResponse & GetOKResponse() const { return this->mOkResult; }
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

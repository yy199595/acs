//
// Created by yy on 2025/2/10.
//

#include "MysqlProto.h"
#include "Util/Crypt/sha1.h"

#include <utility>
#ifdef __OS_WIN__
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif
namespace mysql
{
	Request::Request(std::string sql)
			: mMessage(std::move(sql)), mCmd(mysql::cmd::QUERY)
	{
		this->mRpcId = 0;
		this->mIndex = 0;
	}

	Request::Request(const char* sql, size_t size)
		: mMessage(sql, size),  mCmd(mysql::cmd::QUERY)
	{
		this->mRpcId = 0;
		this->mIndex = 0;
	}

	Request::Request(char cmd, std::string sql)
		: mMessage(std::move(sql)), mCmd(cmd)
	{
		this->mRpcId = 0;
		this->mIndex = 0;
	}

	int Request::OnSendMessage(std::ostream& os)
	{
		uint32_t length = this->mMessage.size() + 1;
		std::vector<uint8_t> length_bytes = mysql::encode(length);
		os.write((char *)length_bytes.data(), length_bytes.size());
		os << this->mIndex << this->mCmd;

		os.write(this->mMessage.c_str(), this->mMessage.size());
		return 0;
	}

	void Request::Clear()
	{
		this->mCmd = 0;
		this->mMessage.clear();
	}
}

namespace mysql
{
	inline std::string mysql_native_password(const std::string & password, const std::string & salt)
	{
		std::string password_hash = help::Sha1::GetHash(password);
		std::string stage1 = help::Sha1::GetHash(password_hash);
		std::string salt_stage1 = salt + stage1;
		std::string challenge_response = help::Sha1::GetHash(salt_stage1);

		for (size_t i = 0; i < password_hash.size(); ++i) {
			challenge_response[i] ^= password_hash[i];
		}

		return challenge_response;
	}
}

namespace mysql
{
	LoginRequest::LoginRequest()
	{
		this->mCharset = 255;
	}

	int LoginRequest::OnSendMessage(std::ostream& os)
	{

		std::string encrypted_password = mysql::mysql_native_password(this->mPassword, this->mSalt);

		std::vector<uint8_t> packet;

		uint32_t client_capabilities =
				0x00000001 |  // CLIENT_LONG_PASSWORD
				0x00000002 |  // CLIENT_FOUND_ROWS
				0x00000004 |  // CLIENT_LONG_FLAG
				0x00000200 |  // CLIENT_PROTOCOL_41（支持 4.1 及以上版本协议）
				0x00000800 |  // CLIENT_SECURE_CONNECTION
				0x00020000 |  // CLIENT_PLUGIN_AUTH
				0x00040000;   // CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA
		if(!this->mDatabase.empty())
		{
			client_capabilities |= 0x00000008;
		}

		uint32_t capabilities_le = htonl(client_capabilities);
		packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&capabilities_le),
				reinterpret_cast<uint8_t*>(&capabilities_le) + 4);


		uint32_t max_packet_size = mysql::config::PACKAGE_MAX_SIZE;
		packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&max_packet_size),
				reinterpret_cast<uint8_t*>(&max_packet_size) + 4);

		packet.emplace_back(this->mCharset);

		packet.insert(packet.end(), 23, 0);
		packet.insert(packet.end(), this->mUser.begin(), this->mUser.end());
		packet.emplace_back(0);  // 用户名以 null 结尾
		if (!encrypted_password.empty()) {
			packet.emplace_back(encrypted_password.size());  // 插入密码长度
			packet.insert(packet.end(), encrypted_password.begin(), encrypted_password.end());
		} else {
			packet.emplace_back(0);  // 空密码情况
		}

		if (!this->mDatabase.empty()) {
			packet.insert(packet.end(), this->mDatabase.begin(), this->mDatabase.end());
			packet.emplace_back(0);  // 以 null 结尾
		}

		packet.insert(packet.end(), this->mAuthPlugin.begin(), this->mAuthPlugin.end());
		packet.emplace_back(0);  // 以 null 结尾


		uint32_t packet_length = packet.size();
		uint8_t sequence_id = 1; // 登录包的 sequence_id 必须是 1

		std::vector<uint8_t> final_packet;
		final_packet.emplace_back(packet_length & 0xFF);
		final_packet.emplace_back((packet_length >> 8) & 0xFF);
		final_packet.emplace_back((packet_length >> 16) & 0xFF);
		final_packet.emplace_back(sequence_id);

		final_packet.insert(final_packet.end(), packet.begin(), packet.end());

		os.write((const char *)final_packet.data(), final_packet.size());
		return 0;
	}
}

namespace mysql
{
	Response::Response()
	{
		this->Clear();
	}

	void Response::Clear()
	{
		this->mIndex = 0;
		this->mLength = 0;
		this->mErrorCode = 0;
		this->mPackageCode = 0;
		this->mMessage.clear();
		this->mResult.contents.clear();
		this->mDecodeStatus = tcp::Decode::None;
		std::memset(&this->mOkResult, 0, sizeof(this->mOkResult));
	}

	int Response::OnRecvMessage(std::istream& os, size_t size)
	{
		switch(this->mDecodeStatus)
		{
			case tcp::Decode::None:
			{
				if(size < 4)
				{
					return 4;
				}
				this->mLength = 0;
				char buffer[4] = { 0 };
				os.readsome((char *)buffer, sizeof(buffer));
				this->mIndex = (unsigned char)buffer[3];
				this->mDecodeStatus = tcp::Decode::MessageHead;
				tcp::Data::Read((char *)buffer, this->mLength, 3, false);
				return this->mLength;
			}
			case tcp::Decode::MessageHead:
			{
				std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);
				{
					os.readsome(buffer.get(), size);
				}
				return this->OnMessage(buffer.get(), size);
			}
			case tcp::Decode::MessageBody:
			{
				this->mMessage.resize(size);
				os.readsome((char*)this->mMessage.c_str(), size);
				break;
			}
			default:
				return tcp::ReadDecodeError;
		}
		return 0;
	}

	unsigned int Response::DecodeColumnCount(unsigned int &pos)
	{
		unsigned int len = (unsigned char)this->mMessage[pos++];
		switch (len)
		{
			case 0XFC:
				tcp::Data::Read(this->mMessage.c_str() + pos, len, 2, false);
				pos += 2;
				break;
			case 0XFD:
				tcp::Data::Read(this->mMessage.c_str() + pos, len, 4, false);
				pos += 4;
				break;
			case 0XFE:
				tcp::Data::Read(this->mMessage.c_str() + pos, len, 8, false);
				pos += 8;
				break;
			default:
			{
				if (len < 0XFB)
				{
					return len;
				}
			}
		}
		return len;
	}

	std::string Response::ReadString(unsigned int & pos)
	{
		std::string result;
		unsigned int len = this->DecodeColumnCount(pos);
		if(len <= 0 || len + pos > this->mMessage.size())
		{
			return result;
		}
		result.assign(this->mMessage.c_str() + pos, len);
		pos += len;
		return result;
	}

	unsigned char Response::ReadChar(unsigned int& pos)
	{
		unsigned char result = 0;
		if(pos < this->mMessage.size())
		{
			result = this->mMessage[pos];
			pos++;
		}
		return result;
	}

	unsigned short Response::ReadShort(unsigned int& pos)
	{
		unsigned short result = 0;
		if(pos < this->mMessage.size())
		{
			tcp::Data::Read(this->mMessage.data() + pos, result, 2, false);
			pos += 2;
		}
		return result;
	}

	int Response::OnMessage(const char * buffer, size_t size)
	{
		int offset = 0;
		this->mPackageCode = buffer[offset++];
		switch(this->mPackageCode)
		{
			case mysql::PACKAGE_OK:
			{
				if (this->mMessage.size() >= 7)
				{
					this->mOkResult.mAffectedRows = (unsigned int)this->mMessage[offset++];
					this->mOkResult.mLastInsertId = (unsigned int)this->mMessage[offset++];
					tcp::Data::Read(this->mMessage.c_str() + 2, this->mOkResult.mServerStatus, offset, false);
					offset +=2;
					tcp::Data::Read(this->mMessage.c_str() + 4, this->mOkResult.mWarningCount, offset, false);
				}
				return tcp::ReadDone;
			}
			case mysql::PACKAGE_ERR:
			{
				tcp::Data::Read(buffer + offset, this->mErrorCode);
				offset += sizeof(this->mErrorCode);

				offset += 6;
				this->mMessage.assign(buffer + offset, size - offset);
				return tcp::ReadDone;
			}
			case mysql::PACKAGE_EOF:
				return tcp::ReadDone;
			default:
				this->mMessage.assign(buffer, size);
				return tcp::ReadPause;
		}
	}
}
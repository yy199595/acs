//
// Created by yy on 2025/2/10.
//

#include "MysqlProto.h"
#include "Util/Crypt/sha1.h"
#include "XCode/XCode.h"
#include "Util/Tools/String.h"
#include "Yyjson/Document/Document.h"
#ifdef __ENABLE_OPEN_SSL__
#include "openssl/sha.h"
#endif
#include <utility>
#ifdef __OS_WIN__
#else
#include <arpa/inet.h>
#endif
namespace mysql
{
	Request::Request(char cmd)
		: mCmd(cmd)
	{
		this->mRpcId = 0;
		this->mIndex = 0;
	}

	Request::Request(const std::string & sql)
			: mMessage(sql), mCmd(mysql::cmd::QUERY)
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

	bool Request::GetCommand(std::string& cmd) const
	{
		size_t pos = this->mMessage.find(' ');
		if(pos == std::string::npos)
		{
			return false;
		}
		cmd = this->mMessage.substr(0, pos);
		help::Str::Toupper(cmd);
		return true;
	}

	int Request::OnSendMessage(std::ostream& os)
	{
		uint32_t length = this->mMessage.size() + 1;
		std::vector<uint8_t> length_bytes = mysql::encode(length);
		os.write((char *)length_bytes.data(), (int)length_bytes.size());
		os << this->mIndex << this->mCmd;

		os.write(this->mMessage.c_str(), (int)this->mMessage.size());
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
	namespace plugin
	{
		std::string native_password(const std::string& password, const std::string& salt)
		{
			std::string password_hash = help::Sha1::GetHash(password);
			std::string stage1 = help::Sha1::GetHash(password_hash);
			std::string salt_stage1 = salt + stage1;
			std::string challenge_response = help::Sha1::GetHash(salt_stage1);

			for (size_t i = 0; i < password_hash.size(); ++i)
			{
				challenge_response[i] ^= password_hash[i];
			}

			return challenge_response;
		}
#ifdef __ENABLE_OPEN_SSL__
		std::string caching_sha2_password(const std::string & password, const std::string & salt)
		{
			unsigned char buf1[32];
			unsigned char buf2[52];
			int i;

			// SHA256( password ) ^ SHA256( SHA256( SHA256( password ) ) + seed)
			SHA256((unsigned char *)password.c_str(), password.size(), buf1);
			SHA256(buf1, 32, buf2);
			memcpy(buf2 + 32, salt.c_str(), salt.size());
			SHA256(buf2, 52, buf2);
			for (i = 0; i < 32; i++)
				buf1[i] ^= buf2[i];

			return std::string((const char *)buf1, 32);
		}
#endif
	}
}

namespace mysql
{
	int LoginRequest::Encode(std::vector<uint8_t>& packet) const
	{
		  uint32_t client_capabilities =
            mysql::client_flag::CLIENT_LONG_PASSWORD |
            mysql::client_flag::CLIENT_LONG_FLAG |
            mysql::client_flag::CLIENT_FOUND_ROWS |
            mysql::client_flag::CLIENT_PROTOCOL_41 |
            mysql::client_flag::CLIENT_MULTI_RESULTS |
            mysql::client_flag::CLIENT_LOCAL_FILES |
            mysql::client_flag::CLIENT_MULTI_STATEMENTS |
            mysql::client_flag::CLIENT_SECURE_CONNECTION |
            mysql::client_flag::CLIENT_TRANSACTIONS |
            mysql::client_flag::CLIENT_PS_MULTI_RESULTS |
			mysql::client_flag::CLIENT_DEPRECATE_EOF;


        std::string encrypted_password;
        if (!this->password.empty())
        {
	        if (this->authPlugin == mysql::plugin::MYSQL_CLEAR_PASSWORD)
	        {
	        	encrypted_password = this->password;
	        	encrypted_password.push_back('\0');
	        }
	        else if (this->authPlugin == mysql::plugin::MYSQL_NATIVE_PASSWORD)
	        {
	        	encrypted_password = mysql::plugin::native_password(this->password, this->salt);
	        }
#ifdef __ENABLE_OPEN_SSL__
	        else if (this->authPlugin == mysql::plugin::CACHING_SHA2_PASSWORD)
	        {
	        	//client_capabilities |= mysql::client_flag::CLIENT_SSL;
	        	client_capabilities |= mysql::client_flag::CLIENT_PLUGIN_AUTH;
	        	encrypted_password = mysql::plugin::caching_sha2_password(this->password, this->salt);
	        }
#endif
	        else
	        {
	        	return XCode::AuthPluginNonsupport;
	        }
        }

        if (!this->database.empty())
        {
            client_capabilities |= mysql::client_flag::CLIENT_CONNECT_WITH_DB;
        }
		//client_capabilities = 260047;
        packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&client_capabilities),
                      reinterpret_cast<uint8_t*>(&client_capabilities) + 4);


        uint32_t max_packet_size = mysql::config::PACKAGE_MAX_SIZE;
        packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&max_packet_size),
                      reinterpret_cast<uint8_t*>(&max_packet_size) + 4);

        packet.emplace_back(this->charset);

        packet.insert(packet.end(), 23, 0);
        packet.insert(packet.end(), this->user.begin(), this->user.end());
        packet.emplace_back(0); // 用户名以 null 结尾
        if (!encrypted_password.empty())
        {
            packet.emplace_back(encrypted_password.size()); // 插入密码长度
            packet.insert(packet.end(), encrypted_password.begin(), encrypted_password.end());
        }
        else
        {
            packet.emplace_back(0); // 空密码情况
        }

        if (!this->database.empty())
        {
            packet.insert(packet.end(), this->database.begin(), this->database.end());
            packet.emplace_back(0); // 以 null 结尾
        }

        packet.insert(packet.end(), this->authPlugin.begin(), this->authPlugin.end());
        packet.emplace_back(0); // 以 null 结尾
		return  XCode::Ok;
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
				if(os.readsome((char *)buffer, sizeof(buffer)) != sizeof(buffer))
				{
					return tcp::read::decode_error;
				}
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
				return tcp::read::decode_error;
		}
		return 0;
	}

	std::string Response::ToString() const
	{
		json::w::Document document;
		if(this->IsOk())
		{
			document.Add("code", "ok");
			document.Add("affected_rows", this->mOkResult.mAffectedRows);
			document.Add("server_status", this->mOkResult.mServerStatus);
			document.Add("warning_count", this->mOkResult.mWarningCount);
			document.Add("last_insert_id", this->mOkResult.mLastInsertId);
			std::unique_ptr<json::w::Value> jsonValue = document.AddArray("result");
			if(this->mResult.contents.empty())
			{
				for(const std::string & json : this->mResult.contents)
				{
					jsonValue->AddObject(json.c_str(), json.size());
				}
			}
		}
		else
		{
			document.Add("code", "error");
			document.Add("error", this->mMessage);
		}
		return document.JsonString();
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

	bool Response::GetFirstResult(std::string& result) const
	{
		if(this->mResult.contents.empty())
		{
			return false;
		}
		result = this->mResult.contents.front();
		return true;
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
		unsigned int offset = 0;
		this->mPackageCode = buffer[offset++];
		switch(this->mPackageCode)
		{
			case mysql::PACKAGE_OK:
			{
				if(size >= 7)
				{
					this->mOkResult.mAffectedRows = (unsigned int)buffer[offset++];
					this->mOkResult.mLastInsertId = (unsigned int)buffer[offset++];
					tcp::Data::Read(buffer + offset, this->mOkResult.mServerStatus, 2, false);
					offset += 2;
					tcp::Data::Read(buffer + offset, this->mOkResult.mWarningCount, 2, false);
					offset += 2;
					if(offset < size)
					{
						this->mMessage.assign(buffer + offset, size - offset);
					}
					if((this->mOkResult.mServerStatus & SERVER_MORE_RESULTS_EXISTS) != 0)
					{
						return tcp::read::pause;
					}
				}
				return tcp::read::done;
			}
			case mysql::PACKAGE_ERR:
			{
				tcp::Data::Read(buffer + offset, this->mErrorCode);
				offset += sizeof(this->mErrorCode);

				offset += 6;
				this->mMessage.assign(buffer + offset, size - offset);
				return tcp::read::done;
			}
			case mysql::PACKAGE_EOF:
			{
				unsigned short warnCount = 0;
				unsigned short statusFlags = 0;
				tcp::Data::Read(buffer + offset, warnCount, 2, false);
				offset += sizeof(warnCount);
				tcp::Data::Read(buffer + offset, statusFlags, 2, false);
				return tcp::read::done;
			}
			default:
				this->mMessage.assign(buffer, size);
				return tcp::read::pause;
		}
	}
}
//
// Created by 64658 on 2025/2/18.
//

#include "PgsqlCommon.h"
#include "Util/Tools/String.h"
#include "Util/Tools/Math.h"

namespace pgsql
{
	inline void Add(std::string & buffer, const std::string & key, const std::string & value)
	{
		buffer.append(key);
		buffer += '\0';
		buffer.append(value);
		buffer += '\0';
	}

	void StartRequest::Clear()
	{

	}

	int StartRequest::OnSendMessage(std::ostream& os)
	{
		std::string packet;
		char buffer[4] = { 0 };
		unsigned int version = pgsql::VERSION;
		tcp::Data::Write(buffer, version);
		packet.append(buffer, sizeof(buffer));

		pgsql::Add(packet, "user", this->user);
		//pgsql::Add(packet, "database", this->db);
		pgsql::Add(packet, "application_name", "acs");
		packet += '\0';

		std::memset(buffer, 0, sizeof(buffer));
		unsigned int len = packet.size() + sizeof(buffer);

		tcp::Data::Write(buffer, len);

		os.write(buffer, sizeof(buffer));
		os.write(packet.c_str(), packet.size());
		return 0;
	}
}

namespace pgsql
{
	void Request::Clear()
	{

	}

	int Request::OnSendMessage(std::ostream& os)
	{
		if(this->mMessage.back() != '\0')
		{
			this->mMessage += '\0';
		}
		unsigned int len = this->mMessage.size() + 4;

		os << this->mType;
		char buffer[sizeof(len)] = { 0};
		tcp::Data::Write(buffer, len);
		os.write(buffer, sizeof(buffer));
		os.write(this->mMessage.c_str(), this->mMessage.size());
		return 0;
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
}

namespace pgsql
{
	Result::Result()
	{
		this->Clear();
	}

	void Result::Clear()
	{
		this->mType = 0;
		this->mRows.clear();
		this->mError.clear();
		this->mStatus.clear();
		this->mFields.clear();
		this->mBuffer.clear();
		this->mResult.clear();
		this->mDecodeStatus = tcp::Decode::ProtoHead;
	}

	int Result::OnSendMessage(std::ostream& os)
	{
		return 0;
	}

	int Result::OnRecvMessage(std::istream& os, size_t size)
	{
		switch(this->mDecodeStatus)
		{
			case tcp::Decode::ProtoHead:
			{
				char buffer[4] = { 0 };
				this->mType = (char)os.get();
				os.readsome(buffer, sizeof(buffer));
				unsigned int len = pgsql::ReadLength(buffer);
				this->mDecodeStatus = tcp::Decode::MessageBody;
				return len - sizeof(buffer);
			}
			case tcp::Decode::MessageBody:
			{
				this->mBuffer.clear();
				this->mBuffer.resize(size);
				os.readsome((char *)this->mBuffer.c_str(), size);
				break;
			}
		}
		switch(this->mType)
		{
			case pgsql::type::error:
			{
				std::vector<std::string> result;
				this->mError.assign(this->mBuffer);
				if(help::Str::Split(this->mBuffer, '\0', result) > 0)
				{
					json::w::Document document;
					for (size_t index = 0; index < result.size() - 1; index += 2)
					{
						const std::string& key = result[index];
						const std::string& value = result[index + 1];
						document.Add(key.c_str(), value);
					}
					document.Encode(&this->mError);
				}
				break;
			}
			case pgsql::type::status:
			{
				std::vector<std::string> result;
				help::Str::Split(this->mBuffer, '\0', result);
				if (result.size() % 2 == 0)
				{
					for (size_t index = 0; index < result.size(); index += 2)
					{
						const std::string & key = result[index];
						const std::string & value = result[index + 1];
						this->mStatus.emplace(key, value);
					}
				}
				break;
			}
			case pgsql::type::data_row:
			{
				this->mRows.emplace_back(this->mBuffer);
				break;
			}
			case pgsql::type::row_description:
			{
				unsigned short len = 0;
				unsigned int offset = 2;
				tcp::Data::Read(this->mBuffer.c_str(), len);
				for (unsigned short index = 0; index < len; index++)
				{
					size_t pos = this->mBuffer.find('\0', offset);
					if (pos != std::string::npos)
					{
						pgsql::FieldInfo fieldInfo;
						fieldInfo.name = this->mBuffer.substr(offset, pos - offset);
						offset += (fieldInfo.name.size() + 1);
						tcp::Data::Read(this->mBuffer.c_str() + offset + 6, fieldInfo.type);
						offset += 18;
						this->mFields.emplace_back(fieldInfo);
					}
				}
				break;
			}
			case pgsql::type::command_complete:
			{
				this->mResult = this->mBuffer;
				if(this->mResult.back() == '\0')
				{
					this->mResult.pop_back();
				}
				break;
			}
		}
		this->mDecodeStatus = tcp::Decode::ProtoHead;
		return 0;
	}

	void Result::DecodeData(pgsql::Response& response)
	{
		response.mError = this->mError;
		if(!this->mResult.empty())
		{
			std::vector<std::string> list;
			if(help::Str::Split(this->mResult, ' ', list) > 0)
			{
				response.mCmd = list.at(0);
				if (list.size() == 2)
				{
					std::string& val = list.at(1);
					help::Math::ToNumber(val, response.count);
				}
				else if (list.size() == 3)
				{
					std::string& val = list.at(2);
					help::Math::ToNumber(val, response.count);
				}
			}
		}
		for(const std::string & message : this->mRows)
		{
			unsigned short len = 0;
			unsigned int offset = 2;
			json::w::Document document;
			tcp::Data::Read(message.c_str(), len);
			for(unsigned short index = 0; index < len; index++)
			{
				int length = 0;
				tcp::Data::Read(message.c_str() + offset, length);
				const FieldInfo& fieldInfo = this->mFields.at(index);
				offset += 4;
				if(length > 0)
				{
					std::string value = message.substr(offset, length);
					offset += length;
					switch(fieldInfo.type)
					{
						case pgsql::field::BOOL:
						{
							break;
						}
						case pgsql::field::NUMBER_INT16:
						case pgsql::field::NUMBER_INT32:
						case pgsql::field::NUMBER_INT64:
						{
							long long number = std::stoll(value);
							document.Add(fieldInfo.name.c_str(), number);
							break;
						}
						case pgsql::field::NUMBER_FLOAT32:
						case pgsql::field::NUMBER_FLOAT64:
						{
							double number = std::stod(value);
							document.Add(fieldInfo.name.c_str(), number);
							break;
						}
						case pgsql::field::STRING_JSON:
						{
							document.AddObject(fieldInfo.name.c_str(), value);
							break;
						}
						default:
						{
							document.Add(fieldInfo.name.c_str(), value);
							break;
						}
					}
				}
			}
			response.mResults.emplace_back(document.JsonString());
		}

	}
}
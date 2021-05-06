#include"JsonHelper.h"

namespace SoEasy
{
	bool RapidJsonWriter::StartAddParameter()
	{
		if (this->jsonWriter)
		{
			delete this->jsonWriter;
			this->jsonWriter = nullptr;
		}
		this->strBuf.Clear();
		this->jsonWriter = new rapidjson::Writer<rapidjson::StringBuffer>(strBuf);
		return this->jsonWriter->StartObject();
	}

	bool RapidJsonWriter::AddParameter(const char * key, const int value)
	{
		if (this->jsonWriter != nullptr)
		{
			jsonWriter->Key(key);
			jsonWriter->Int(value);
			return true;
		}
		return false;
	}

	bool RapidJsonWriter::AddParameter(const char * key, const double value)
	{
		if (this->jsonWriter != nullptr)
		{
			jsonWriter->Key(key);
			jsonWriter->Double(value);
			return true;
		}
		return false;
	}

	bool RapidJsonWriter::AddParameter(const char * key, const char * value)
	{
		if (this->jsonWriter != nullptr)
		{
			this->jsonWriter->Key(key);
			this->jsonWriter->String(value);
			return true;
		}
		return false;
	}

	bool RapidJsonWriter::AddParameter(const char * key, const long long value)
	{
		if (this->jsonWriter != nullptr)
		{
			jsonWriter->Key(key);
			jsonWriter->Int64(value);
			return true;
		}
		return false;
	}

	bool RapidJsonWriter::AddParameter(const char * key, const std::string value)
	{
		if (this->jsonWriter != nullptr)
		{
			jsonWriter->Key(key);
			jsonWriter->String(value.c_str(), value.length());
			return true;
		}
		return false;
	}

	bool RapidJsonWriter::AddParameter(const char * key, const unsigned int value)
	{
		if (this->jsonWriter != nullptr)
		{
			jsonWriter->Key(key);
			jsonWriter->Uint(value);
			return true;
		}
		return false;
	}

	bool RapidJsonWriter::AddParameter(const char * key, const unsigned long long value)
	{
		if (this->jsonWriter != nullptr)
		{
			jsonWriter->Key(key);
			jsonWriter->Uint64(value);
			return true;
		}
		return false;
	}

	bool RapidJsonWriter::AddParameter(const char * key, const std::vector<std::string> value)
	{
		if (this->jsonWriter != nullptr)
		{
			jsonWriter->Key(key);
			jsonWriter->StartArray();
			for (size_t index = 0; index < value.size(); index++)
			{
				const std::string & str = value[index];
				jsonWriter->String(str.c_str(), str.size());
			}
			jsonWriter->EndArray();
			return true;
		}
		return false;
	}


	const std::string & RapidJsonWriter::Serialization()
	{
		this->jsonWriter->EndObject();
		const char * str = this->strBuf.GetString();
		const size_t lenght = this->strBuf.GetSize();
		mTempString.clear();
		mTempString.append(str, lenght);
		delete this->jsonWriter;
		this->jsonWriter = nullptr;
		return mTempString;
	}
}

namespace SoEasy
{
	bool RapidJsonReader::TryParse(const char * str, size_t size)
	{
		if (!document.Parse(str, size).HasParseError())
		{
			this->mJsonString = "";
			this->mJsonString.append(str, size);
			return true;
		}
		return false;
	}
	
	bool RapidJsonReader::TryParse(const std::string & str)
	{
		if (!document.Parse(str.c_str(), str.size()).HasParseError())
		{
			this->mJsonString = "";
			this->mJsonString.append(str);
			return true;
		}
		return false;
	}
	bool RapidJsonReader::TryGetValue(const char * key, int & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsInt())
		{
			data = iter->value.GetInt();
			return true;
		}
		return false;
	}
	bool RapidJsonReader::TryGetValue(const char * key, bool & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsBool())
		{
			data = iter->value.GetBool();
			return true;
		}
		return false;
	}
	bool RapidJsonReader::TryGetValue(const char * key, short & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsNumber())
		{
			data = (short)iter->value.GetInt();
			return true;
		}
		return false;
	}

	bool RapidJsonReader::TryGetValue(const char * key, unsigned short & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsNumber())
		{
			data = (unsigned short)iter->value.GetInt();
			return true;
		}
		return false;
	}

	bool RapidJsonReader::TryGetValue(const char * key, float & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsFloat())
		{
			data = iter->value.GetFloat();
			return true;
		}
		return false;
	}
	bool RapidJsonReader::TryGetValue(const char * key, double & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsDouble())
		{
			data = iter->value.GetDouble();
			return true;
		}
		return false;
	}
	bool RapidJsonReader::TryGetValue(const char * key, unsigned int & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsUint())
		{
			data = iter->value.GetUint();
			return true;
		}
		return false;
	}
	bool RapidJsonReader::TryGetValue(const char * key, long long & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsInt64())
		{
			data = iter->value.GetInt64();
			return true;
		}
		return false;
	}
	bool RapidJsonReader::TryGetValue(const char * key, unsigned long long & data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsUint64())
		{
			data = iter->value.GetUint64();
			return true;
		}
		return false;
	}
	bool RapidJsonReader::TryGetValue(const char * key, std::vector<std::string>& data)
	{
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsArray())
		{
			for (unsigned int index = 0; index < iter->value.Size(); index++)
			{
				if (iter->value[index].IsString())
				{
					const char * str = iter->value[index].GetString();
					const size_t size = iter->value[index].GetStringLength();
					data.push_back(std::string(str, size));
				}
			}
			return true;
		}
		return false;
	}

	bool RapidJsonReader::TryGetValue(const char * key, std::string & data)
	{
		data = "";
		MemberIter iter = document.FindMember(key);
		if (iter != document.MemberEnd() && iter->value.IsString())
		{
			data.append(iter->value.GetString(), iter->value.GetStringLength());
			return true;
		}
		return false;
	}
}
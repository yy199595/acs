//
// Created by yjz on 2022/3/26.
//
#include"JsonWriter.h"

namespace Json
{
	Writer& Writer::operator<<(std::vector<int>& value)
	{
		this->mJsonWriter.StartArray();
		for(const int val : value)
		{
			this->mJsonWriter.Int(val);
		}
		this->mJsonWriter.EndArray();
		return *this;
	}

	Writer& Writer::operator<<(std::vector<std::string>& value)
	{
		this->mJsonWriter.StartArray();
		for(const std::string & val : value)
		{
			this->mJsonWriter.String(val.c_str(), val.size());
		}
		this->mJsonWriter.EndArray();
		return *this;
	}
}

namespace Json
{
	Writer::Writer(bool isObj)
		: mIsObject(isObj), mJsonWriter(this->mStringBuf)
	{
		if (this->mIsObject)
		{
			this->mJsonWriter.StartObject();
			return;
		}
		this->mJsonWriter.StartArray();
	}


	bool Writer::AddMember(const char* key, int value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Int(value);
	}
	bool Writer::AddMember(const char* key, bool value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Bool(value);
	}
	bool Writer::AddMember(const char* key, short value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Int(value);
	}
	bool Writer::AddMember(const char* key, float value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Double(value);
	}
	bool Writer::AddMember(const char* key, double value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Double(value);
	}
	bool Writer::AddMember(const char* key, long long int value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Int64(value);
	}
	bool Writer::AddMember(const char* key, unsigned int value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Uint(value);
	}
	bool Writer::AddMember(const char* key, unsigned short value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Uint(value);
	}
	bool Writer::AddMember(const char* key, const std::string& value)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.String(value.c_str(), value.size());
	}
	bool Writer::AddMember(const char* key, const std::vector<int>& value)
	{
		if (!this->StartArray(key))
		{
			return false;
		}
		for (const int member : value)
		{
			this->mJsonWriter.Int(member);
		}
		return this->mJsonWriter.EndArray();
	}
	bool Writer::AddMember(const char* key, const std::vector<long long int>& value)
	{
		if (!this->StartArray(key)) return false;
		for (const long long member : value)
		{
			this->mJsonWriter.Int64(member);
		}
		return this->mJsonWriter.EndArray();
	}
	bool Writer::AddMember(const char* key, const std::vector<std::string>& value)
	{
		if (!this->StartArray(key)) return false;
		for (const std::string& member : value)
		{
			this->mJsonWriter.String(member.c_str(), member.size());
		}
		return this->mJsonWriter.EndArray();
	}
	bool Writer::StartArray()
	{
		return this->mJsonWriter.StartArray();
	}
	bool Writer::StartObject()
	{
		return this->mJsonWriter.StartObject();
	}
	bool Writer::StartArray(const char* key)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.StartArray();
	}
	bool Writer::StartObject(const char* key)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.StartObject();
	}
	bool Writer::EndObject()
	{
		return this->mJsonWriter.EndObject();
	}
	bool Writer::EndArray()
	{
		return this->mJsonWriter.EndArray();
	}
	const std::string Writer::ToJsonString()
	{
		if (this->mJsonWriter.IsComplete())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return std::string(str, size);
		}
		if (this->mIsObject && this->mJsonWriter.EndObject())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return std::string(str, size);
		}
		else if (this->mJsonWriter.EndArray())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return std::string(str, size);
		}
		return std::string();
	}
	bool Writer::AddMember(const char* key, const char* str, size_t size)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.String(str, size);
	}
	bool Writer::AddMember(const char* key, const char* str)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.String(str);
	}

	size_t Writer::WriterStream(std::string& os)
	{
		if (this->mJsonWriter.IsComplete())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.append(str, size);
			return size;
		}
		if (this->mIsObject && this->mJsonWriter.EndObject())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.append(str, size);
			return size;
		}
		else if (this->mJsonWriter.EndArray())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.append(str, size);
			return size;
		}
		return 0;
	}

	size_t Writer::WriterStream(std::ostream& os)
	{
		if (this->mJsonWriter.IsComplete())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.write(str, size);
			return size;
		}
		if (this->mIsObject && this->mJsonWriter.EndObject())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.write(str, size);
			return size;
		}
		else if (this->mJsonWriter.EndArray())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.write(str, size);
			return size;
		}
		return 0;
	}
	size_t Writer::GetJsonSize()
	{
		if (this->mJsonWriter.IsComplete())
		{
			return this->mStringBuf.GetSize();
		}
		if (this->mIsObject && this->mJsonWriter.EndObject())
		{
			return this->mStringBuf.GetSize();
		}
		else if (this->mJsonWriter.EndArray())
		{
			return this->mStringBuf.GetSize();
		}
		return 0;
	}
	bool Writer::AddMember(const char* key, XCode code)
	{
		return this->mJsonWriter.String(key)
			   && this->mJsonWriter.Int((int)code);
	}

	bool Writer::AddMember(long long value)
	{
		return this->mJsonWriter.Int64(value);
	}

	bool Writer::AddMember(const std::string& value)
	{
		return this->mJsonWriter.String(value.c_str(), value.size());
	}

	bool Writer::GetDocument(rapidjson::Document& jsonDocument)
	{
		if (this->mJsonWriter.IsComplete())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		if (this->mIsObject && this->mJsonWriter.EndObject())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		else if (this->mJsonWriter.EndArray())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		return false;
	}
}

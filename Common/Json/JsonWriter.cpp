//
// Created by yjz on 2022/3/26.
//
#include"JsonWriter.h"

namespace Json
{
	Writer& Writer::operator<<(JsonType type)
	{
		switch(type)
		{
		case JsonType::StartObject:
			this->StartObject();
			break;
		case JsonType::EndObject:
			this->EndObject();
			break;
		case JsonType::StartArray:
			this->StartArray();
			break;
		case JsonType::EndArray:
			this->EndArray();
			break;
		}
		return *this;
	}

	Writer& Writer::operator<<(std::vector<int>& value)
	{
		this->StartArray();
		for(const int val : value)
		{
			this->Int(val);
		}
		this->EndArray();
		return *this;
	}

	Writer& Writer::operator<<(std::vector<std::string>& value)
	{
		this->StartArray();
		for(const std::string & val : value)
		{
			this->String(val.c_str(), val.size());
		}
		this->EndArray();
		return *this;
	}

	Writer& Writer::operator<<(std::list<std::string>& value)
	{
		this->StartArray();
		for(const std::string & val : value)
		{
			this->String(val.c_str(), val.size());
		}
		this->EndArray();
		return *this;
	}
}

namespace Json
{
	Writer::Writer(bool isObj)
		: mIsObject(isObj), rapidjson::Writer<rapidjson::StringBuffer>(this->mStringBuf)
	{
		if (this->mIsObject)
		{
			this->StartObject();
			return;
		}
		this->StartArray();
	}



	const std::string Writer::ToJsonString()
	{
		if (this->IsComplete())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return std::string(str, size);
		}
		if (this->mIsObject && this->EndObject())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return std::string(str, size);
		}
		else if (this->EndArray())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return std::string(str, size);
		}
		return std::string();
	}

	size_t Writer::WriterStream(std::string& os)
	{
		if (this->IsComplete())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.append(str, size);
			return size;
		}
		if (this->mIsObject && this->EndObject())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.append(str, size);
			return size;
		}
		else if (this->EndArray())
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
		if (this->IsComplete())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.write(str, size);
			return size;
		}
		if (this->mIsObject && this->EndObject())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			os.write(str, size);
			return size;
		}
		else if (this->EndArray())
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
		if (this->IsComplete())
		{
			return this->mStringBuf.GetSize();
		}
		if (this->mIsObject && this->EndObject())
		{
			return this->mStringBuf.GetSize();
		}
		else if (this->EndArray())
		{
			return this->mStringBuf.GetSize();
		}
		return 0;
	}

	void Writer::AddBinString(const char* str, size_t size)
	{
		this->String(str, size);
	}


	bool Writer::GetDocument(rapidjson::Document& jsonDocument)
	{
		if (this->IsComplete())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		if (this->mIsObject && this->EndObject())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		else if (this->EndArray())
		{
			const char* str = this->mStringBuf.GetString();
			const size_t size = this->mStringBuf.GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		return false;
	}
}

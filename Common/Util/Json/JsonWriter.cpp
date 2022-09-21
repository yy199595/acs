//
// Created by yjz on 2022/3/26.
//
#include"JsonWriter.h"

namespace Json
{
	Writer& Writer::BeginArray(const char* key)
	{
		this->mWriter->String(key);
		this->mWriter->StartArray();
		return *this;
	}

	Writer& Writer::BeginArray()
	{
		this->mWriter->StartArray();
		return *this;
	}

	Writer& Writer::BeginObject()
	{
		this->mWriter->StartObject();
		return *this;
	}

	Writer& Writer::BeginObject(const char* key)
	{
		this->mWriter->String(key);
		this->mWriter->StartObject();
		return *this;
	}

	Writer& Writer::operator<<(Json::End type)
	{
		switch(type)
		{
		case End::EndObject:
			this->mWriter->EndObject();
			break;
		case End::EndArray:
			this->mWriter->EndArray();
			break;
		}
		return *this;
	}

	Writer& Writer::operator<<(std::vector<int>& value)
	{
		for(const int val : value)
		{
			this->mWriter->Int(val);
		}
		this->mWriter->EndArray();
		return *this;
	}

	Writer& Writer::operator<<(std::vector<std::string>& value)
	{
		for(const std::string & val : value)
		{
			this->mWriter->String(val.c_str(), val.size());
		}
		return *this;
	}

	Writer& Writer::operator<<(std::list<std::string>& value)
	{
		for(const std::string & val : value)
		{
			this->mWriter->String(val.c_str(), val.size());
		}
		return *this;
	}
}

namespace Json
{
	Writer::Writer(bool isObj)
		: mIsObject(isObj)
	{
        this->mStringBuf = new rapidjson::StringBuffer();
        this->mWriter = new  rapidjson::Writer<rapidjson::StringBuffer>(*this->mStringBuf);
        //this->mDocument.Accept(this->mWriter);
		if (this->mIsObject)
		{
			this->mWriter->StartObject();
			return;
		}
		this->mWriter->StartArray();
	}

    Writer::~Writer()
    {
        delete this->mWriter;
        delete this->mStringBuf;
    }



	const std::string Writer::JsonString()
	{
		if (this->mWriter->IsComplete())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			return std::string(str, size);
		}
		if (this->mIsObject && this->mWriter->EndObject())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			return std::string(str, size);
		}
		else if (this->mWriter->EndArray())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			return std::string(str, size);
		}
		return std::string();
	}

	size_t Writer::WriterStream(std::string& os)
	{
		if (this->mWriter->IsComplete())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			os.append(str, size);
			return size;
		}
		if (this->mIsObject && this->mWriter->EndObject())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			os.append(str, size);
			return size;
		}
		else if (this->mWriter->EndArray())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			os.append(str, size);
			return size;
		}
		return 0;
	}

	size_t Writer::WriterStream(std::ostream& os)
	{
		if (this->mWriter->IsComplete())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			os.write(str, size);
			return size;
		}
		if (this->mIsObject && this->mWriter->EndObject())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			os.write(str, size);
			return size;
		}
		else if (this->mWriter->EndArray())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			os.write(str, size);
			return size;
		}
		return 0;
	}
	size_t Writer::GetJsonSize()
	{
		if (this->mWriter->IsComplete())
		{
			return this->mStringBuf->GetSize();
		}
		if (this->mIsObject && this->mWriter->EndObject())
		{
			return this->mStringBuf->GetSize();
		}
		else if (this->mWriter->EndArray())
		{
			return this->mStringBuf->GetSize();
		}
		return 0;
	}

	void Writer::AddBinString(const char* str, size_t size)
	{
		this->mWriter->String(str, size);
	}


	bool Writer::GetDocument(rapidjson::Document& jsonDocument)
	{
		if (this->mWriter->IsComplete())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		if (this->mIsObject && this->mWriter->EndObject())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		else if (this->mWriter->EndArray())
		{
			const char* str = this->mStringBuf->GetString();
			const size_t size = this->mStringBuf->GetSize();
			return !jsonDocument.Parse(str, size).HasParseError();
		}
		return false;
	}
}

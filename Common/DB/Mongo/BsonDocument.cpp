//
// Created by mac on 2022/6/27.
//

#include "BsonDocument.h"

namespace Bson
{
	bool ReaderDocument::ParseFromStream(std::istream& is)
	{
		char buffer[128] = { 0 };
		size_t size = is.readsome(buffer, 128);
		while (size > 0)
		{
			this->mBuffer.append(buffer, size);
			size = is.readsome(buffer, 128);
		}
		this->mObject = new Bson::BsonObject(this->mBuffer.c_str());
		return true;
	}

	bool WriterDocument::WriterToStream(std::ostream& os)
	{
		const char* str = this->mBsonBuilder._done();
		const int size = this->mBsonBuilder.len();
		os.write(str, size);
		return true;
	}
	int WriterDocument::GetStreamLength()
	{
		this->mBsonBuilder._done();
		return this->mBsonBuilder.len();
	}

	bool WriterDocument::Add(const char* key, int value)
	{
		this->mBsonBuilder.append(key, value);
		return true;
	}

	bool WriterDocument::Add(const char* key, bool value)
	{
		this->mBsonBuilder.append(key, value);
		return true;
	}

	bool WriterDocument::Add(const char* key, long long value)
	{
		this->mBsonBuilder.append(key, value);
		return true;
	}

	bool WriterDocument::Add(const char* key, const std::string& value)
	{
		this->mBsonBuilder.append(key, value);
		return true;
	}

}
namespace Bson
{
	bool ReaderDocument::Get(const char* key, int& value) const
	{
		if(this->mObject->hasField(key))
		{
			value = this->mObject->getField(key).Int();
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, bool& value) const
	{
		if(this->mObject->hasField(key))
		{
			value = this->mObject->getField(key).Bool();
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, long long& value) const
	{
		if(this->mObject->hasField(key))
		{
			value = this->mObject->getField(key).Long();
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, std::string& value) const
	{
		if(this->mObject->hasField(key))
		{
			value = this->mObject->getField(key).str();
			return true;
		}
		return false;
	}

	void ReaderDocument::WriterToJson(std::string& json)
	{
		json = this->mObject->toString();
	}
}
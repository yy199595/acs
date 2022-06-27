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
		this->mObject = new _bson::bsonobj(this->mBuffer.c_str());
	}

	bool WriterDocument::WriterToStream(std::ostream& os)
	{
		const int size = this->mBsonBuilder.len();
		const char* str = this->mBsonBuilder._done();
		os.write(str, size);
		return true;
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
	bool ReaderDocument::Get(const char* key, int& value)
	{
		if(this->mObject->hasField(key))
		{
			value = this->mObject->getIntField(key);
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, bool& value)
	{
		if(this->mObject->hasField(key))
		{
			value = this->mObject->getIntField(key);
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, long long& value)
	{
		if(this->mObject->hasField(key))
		{
			value = this->mObject->getField(key).Long();
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, std::string& value)
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
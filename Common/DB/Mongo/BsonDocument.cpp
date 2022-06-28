//
// Created by mac on 2022/6/27.
//

#include "BsonDocument.h"

namespace Bson
{
	ReaderDocument::ReaderDocument(const char* bson)
		: _bson::bsonobj(bson)
	{

	}

	bool WriterDocument::WriterToStream(std::ostream& os)
	{
		const char* str = this->_done();
		const int size = this->len();
		os.write(str, size);
		return true;
	}
	int WriterDocument::GetStreamLength()
	{
		this->_done();
		return this->len();
	}

	bool WriterDocument::Add(const char* key, int value)
	{
		this->append(key, value);
		return true;
	}

	bool WriterDocument::Add(const char* key, bool value)
	{
		this->append(key, value);
		return true;
	}

	bool WriterDocument::Add(const char* key, long long value)
	{
		this->append(key, value);
		return true;
	}

	bool WriterDocument::Add(const char* key, const std::string& value)
	{
		this->append(key, value);
		return true;
	}

}
namespace Bson
{
	bool ReaderDocument::Get(const char* key, int& value) const
	{
		if(this->hasField(key))
		{
			value = this->getField(key).Int();
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, bool& value) const
	{
		if(this->hasField(key))
		{
			value = this->getField(key).Bool();
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, long long& value) const
	{
		if(this->hasField(key))
		{
			value = this->getField(key).Long();
			return true;
		}
		return false;
	}

	bool ReaderDocument::Get(const char* key, std::string& value) const
	{
		if(this->hasField(key))
		{
			value = this->getField(key).str();
			return true;
		}
		return false;
	}

	void ReaderDocument::WriterToJson(std::string& json)
	{
		json = this->toString();
	}
}
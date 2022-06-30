//
// Created by mac on 2022/6/27.
//

#include "BsonDocument.h"

namespace Bson
{
	void ArrayDocument::Add(int value)
	{
		this->append(std::to_string(this->mIndex++), value);
	}

	void ArrayDocument::Add(bool value)
	{
		this->append(std::to_string(this->mIndex++), value);
	}
	void ArrayDocument::Add(long long value)
	{
		this->append(std::to_string(this->mIndex++), value);
	}
	void ArrayDocument::Add(const std::string& value)
	{
		this->append(std::to_string(this->mIndex++), value);
	}

	void ArrayDocument::Add(const char* str, size_t size)
	{
		this->append(std::to_string(this->mIndex++), str, size);
	}

	void ArrayDocument::Add(WriterDocument& document)
	{
		_bson::bsonobjbuilder& build = (_bson::bsonobjbuilder&)document;
		_b.appendNum((char)_bson::Object);
		_b.appendStr(std::to_string(this->mIndex++));
		_b.appendBuf(build._done(), build.len());
	}
}

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

	const char* WriterDocument::Serialize(int & length)
	{
		char * bson = this->_done();
		length = this->len();
		return bson;
	}

	void WriterDocument::Add(const char* key, WriterDocument& document)
	{
		_bson::bsonobjbuilder& build = (_bson::bsonobjbuilder&)document;
		_b.appendNum((char)_bson::Object);
		_b.appendStr(key);
		_b.appendBuf(build._done(), build.len());
	}

	void WriterDocument::Add(const char* key, ArrayDocument& document)
	{
		_bson::bsonobjbuilder& build = (_bson::bsonobjbuilder&)document;
		_b.appendNum((char)_bson::Array);
		_b.appendStr(key);
		_b.appendBuf(build._done(), build.len());
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

	_bson::BSONType ReaderDocument::Type(const char* key) const
	{
		if(this->hasField(key))
		{
			return this->getField(key).type();
		}
		return _bson::EOO;
	}

	void ReaderDocument::WriterToJson(std::string& json)
	{
		Json::Writer jsonWriter;
		std::set<std::string> elements;
		this->getFieldNames(elements);
		for(const std::string & key : elements)
		{
			jsonWriter << key;
			this->WriterToJson(this->getField(key), jsonWriter);
		}
		jsonWriter.WriterStream(json);
	}

	void ReaderDocument::WriterToJson(const _bson::bsonelement& bsonelement, Json::Writer& json)
	{
		switch(bsonelement.type())
		{
		case _bson::BSONType::Bool:
			json << bsonelement.Bool();
			break;
		case _bson::BSONType::String:
			json << bsonelement.String();
			break;
		case _bson::BSONType::NumberInt:
			json << bsonelement.Int();
			break;
		case _bson::BSONType::NumberLong:
			json << bsonelement.Long();
			break;
		case _bson::BSONType::NumberDouble:
			json << bsonelement.Double();
			break;
		case _bson::BSONType::BinData:
		{
			int len = 0;
			json.AddBinString(bsonelement.binData(len), len);
		}
			break;
		case _bson::BSONType::Object:
		{
			std::set<std::string> elements;
			json << Json::JsonType::StartObject;
			_bson::bsonobj obj = bsonelement.object();
			obj.getFieldNames(elements);
			for(const std::string & key : elements)
			{
				json << key;
				this->WriterToJson(obj.getField(key), json);
			}
			json << Json::JsonType::EndObject;
		}
			break;
		case _bson::BSONType::Array:
		{
			json << Json::JsonType::StartArray;
			std::set<std::string> elements;
			const _bson::bsonobj obj = bsonelement.object();
			obj.getFieldNames(elements);
			for(const std::string & key : elements)
			{
				this->WriterToJson(obj.getField(key), json);
			}
			json << Json::JsonType::EndArray;
		}
			break;
		case _bson::BSONType::Timestamp:
			json << (long long)bsonelement.timestampValue();
			break;
		case _bson::BSONType::Date:
			json << bsonelement.date().asInt64();
			break;
		default:
			json << bsonelement.toString();
			break;
		}
	}
}
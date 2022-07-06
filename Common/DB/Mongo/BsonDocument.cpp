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

	void ArrayDocument::Add(unsigned int value)
	{
		this->append(std::to_string(this->mIndex++), value);
	}

	void ArrayDocument::Add(double value)
	{
		this->append(std::to_string(this->mIndex++), value);
	}

	void ArrayDocument::Add(Document& document)
	{
		_bson::bsonobjbuilder& build = (_bson::bsonobjbuilder&)document;
		if(document.IsObject())
		{
			_b.appendNum((char)_bson::Object);
		}
		else
		{
			_b.appendNum((char)_bson::Array);
		}
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

	bool ReaderDocument::IsOk() const
	{
		double isOk = 0;
		return this->Get("ok", isOk) && isOk != 0;
	}

	bool ReaderDocument::Get(const char* key, double& value) const
	{
		if(this->hasField(key))
		{
			value = this->getField(key).Double();
			return true;
		}
		return false;
	}

	bool WriterDocument::FromByJson(const std::string& json)
	{
		rapidjson::Document document;
		if(document.Parse(json.c_str(), json.size()).HasParseError())
		{
			return false;
		}
		for(auto iter = document.MemberBegin(); iter != document.MemberEnd(); iter++)
		{
			const char * key = iter->name.GetString();
			if(!this->WriterToBson(key, *this, iter->value))
			{
				return false;
			}
		}
		return true;
	}

	bool WriterDocument::WriterToBson(const char* key, Bson::Document & document, const rapidjson::Value& jsonValue)
	{
		if(jsonValue.IsString())
		{
			const char * str = jsonValue.GetString();
			const size_t size = jsonValue.GetStringLength();
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, str, size);
				return true;
			}
			document.Cast<ArrayDocument>().Add(str, size);
			return true;
		}
		else if(jsonValue.IsBool())
		{
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, jsonValue.GetBool());
				return true;
			}
			document.Cast<ArrayDocument>().Add(jsonValue.GetBool());
			return true;
		}
		else if(jsonValue.IsInt())
		{
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, jsonValue.GetInt());
				return true;
			}
			document.Cast<ArrayDocument>().Add(jsonValue.GetInt());
			return true;
		}
		else if(jsonValue.IsInt64())
		{
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, jsonValue.GetInt64());
				return true;
			}
			document.Cast<ArrayDocument>().Add(jsonValue.GetInt64());
			return true;
		}
		else if(jsonValue.IsUint())
		{
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, jsonValue.GetUint());
				return true;
			}
			document.Cast<ArrayDocument>().Add(jsonValue.GetUint());
			return true;
		}
		else if(jsonValue.IsUint64())
		{
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, (long long)jsonValue.GetUint64());
				return true;
			}
			document.Cast<ArrayDocument>().Add((long long)jsonValue.GetUint64());
			return true;
		}
		else if(jsonValue.IsDouble())
		{
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, jsonValue.GetDouble());
				return true;
			}
			document.Cast<ArrayDocument>().Add(jsonValue.GetDouble());
			return true;
		}
		else if(jsonValue.IsFloat())
		{
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, jsonValue.GetFloat());
				return true;
			}
			document.Cast<ArrayDocument>().Add(jsonValue.GetFloat());
			return true;
		}
		else if(jsonValue.IsObject())
		{
			Bson::WriterDocument obj;
			for(auto iter = jsonValue.MemberBegin(); iter != jsonValue.MemberEnd(); iter++)
			{
				const char * key = iter->name.GetString();
				if(!this->WriterToBson(key, obj, iter->value))
				{
					return false;
				}
			}
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, obj);
				return true;
			}
			document.Cast<ArrayDocument>().Add(obj);
			return true;
		}
		else if(jsonValue.IsArray())
		{
			Bson::ArrayDocument arrayDocument;
			for(int index = 0; index < jsonValue.Size(); index++)
			{
				if(!this->WriterToBson(nullptr, arrayDocument, jsonValue[index]))
				{
					return false;
				}
			}
			if(document.IsObject())
			{
				document.Cast<WriterDocument>().Add(key, arrayDocument);
				return true;
			}
			document.Cast<ArrayDocument>().Add(arrayDocument);
			return true;
		}
		return false;
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

	void WriterDocument::Add(const char* key, Document& document)
	{
		_bson::bsonobjbuilder& build = (_bson::bsonobjbuilder&)document;
		if(document.IsObject())
		{
			_b.appendNum((char)_bson::Object);
		}
		else
		{
			_b.appendNum((char)_bson::Array);
		}
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
			json <<  bsonelement.Bool();
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
			json.BeginObject();
			std::set<std::string> elements;
			_bson::bsonobj obj = bsonelement.object();
			obj.getFieldNames(elements);
			for(const std::string & key : elements)
			{
				json << key;
				this->WriterToJson(obj.getField(key), json);
			}
			json << Json::End::EndObject;
		}
			break;
		case _bson::BSONType::Array:
		{
			json.BeginArray();
			std::set<std::string> elements;
			const _bson::bsonobj obj = bsonelement.object();
			obj.getFieldNames(elements);
			for(const std::string & key : elements)
			{
				this->WriterToJson(obj.getField(key), json);
			}
			json << Json::End::EndArray;
		}
			break;
		case _bson::BSONType::Timestamp:
			json << bsonelement.timestampValue();
			break;
		case _bson::BSONType::Date:
			json << (long long)bsonelement.date().asInt64();
			break;
		default:
			json << bsonelement.toString();
			break;
		}
	}
}
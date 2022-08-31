//
// Created by mac on 2022/6/27.
//

#include "BsonDocument.h"

namespace Bson
{
	namespace Writer
	{
		void Array::Add(int value)
		{
			this->append(std::to_string(this->mIndex++), value);
		}

		void Array::Add(bool value)
		{
			this->append(std::to_string(this->mIndex++), value);
		}

		void Array::Add(long long value)
		{
			this->append(std::to_string(this->mIndex++), value);
		}

		void Array::Add(const std::string& value)
		{
			this->append(std::to_string(this->mIndex++), value);
		}

		void Array::Add(const char* str, size_t size)
		{
			this->append(std::to_string(this->mIndex++), str, size);
		}

		void Array::Add(unsigned int value)
		{
			this->append(std::to_string(this->mIndex++), value);
		}

		void Array::Add(double value)
		{
			this->append(std::to_string(this->mIndex++), value);
		}

		void Array::Add(Document& document)
		{
			_bson::bsonobjbuilder& build = (_bson::bsonobjbuilder&)document;
			if (document.IsObject())
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
}

namespace Bson
{
	namespace Read
	{
		Object::Object(const char* bson)
				: _bson::bsonobj(bson)
		{

		}

		bool Object::IsOk() const
		{
			double isOk = 0;
			return this->Get("ok", isOk) && isOk != 0;
		}

	}
	namespace Writer
	{

        void Object::WriterToJson(std::string &json)
        {
            int length = 0;
            Bson::Read::Object obj(this->Serialize(length));
            obj.WriterToJson(json);
        }


        BsonDocumentType Object::FromByJson(const std::string& json)
		{
			rapidjson::Document document;
			if (document.Parse(json.c_str(), json.size()).HasParseError())
			{
				return BsonDocumentType::None;
			}
            if(document.HasMember("_id"))
            {
                if(document["_id"].IsInt64())
                {
                    long long id = document["_id"].GetInt64();
                    this->mId = std::to_string(id);
                }
                else if(document["_id"].IsString())
                {
                    this->mId = document["_id"].GetString();
                }
            }
            if(document.IsArray())
            {
                for(size_t index = 0; index < document.Size(); index++)
                {
                    std::string key = std::to_string(index);
                    this->WriterToBson(key.c_str(), *this, document[index]);
                }
                return BsonDocumentType::Array;
            }
            if(document.IsObject())
            {
                auto iter = document.MemberBegin();
                for (; iter != document.MemberEnd(); iter++)
                {
                    const char *key = iter->name.GetString();
                    if (!this->WriterToBson(key, *this, iter->value))
                    {
                        return BsonDocumentType::None;
                    }
                }
                return BsonDocumentType::Object;
            }
            return BsonDocumentType::None;
		}

		bool Object::WriterToBson(const char* key, Document& document, const rapidjson::Value& jsonValue)
		{
			if (jsonValue.IsString())
			{
				const char* str = jsonValue.GetString();
				const size_t size = jsonValue.GetStringLength();
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, str, size);
					return true;
				}
				document.Cast<Array>().Add(str, size);
				return true;
			}
			else if (jsonValue.IsBool())
			{
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, jsonValue.GetBool());
					return true;
				}
				document.Cast<Array>().Add(jsonValue.GetBool());
				return true;
			}
			else if (jsonValue.IsInt())
			{
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, jsonValue.GetInt());
					return true;
				}
				document.Cast<Array>().Add(jsonValue.GetInt());
				return true;
			}
			else if (jsonValue.IsInt64())
			{
				long long value = (long long)jsonValue.GetInt64();
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, value);
					return true;
				}
				document.Cast<Array>().Add(value);
				return true;
			}
			else if (jsonValue.IsUint())
			{
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, jsonValue.GetUint());
					return true;
				}
				document.Cast<Array>().Add(jsonValue.GetUint());
				return true;
			}
			else if (jsonValue.IsUint64())
			{
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, (long long)jsonValue.GetUint64());
					return true;
				}
				document.Cast<Array>().Add((long long)jsonValue.GetUint64());
				return true;
			}
			else if (jsonValue.IsDouble())
			{
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, jsonValue.GetDouble());
					return true;
				}
				document.Cast<Array>().Add(jsonValue.GetDouble());
				return true;
			}
			else if (jsonValue.IsFloat())
			{
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, jsonValue.GetFloat());
					return true;
				}
				document.Cast<Array>().Add(jsonValue.GetFloat());
				return true;
			}
			else if (jsonValue.IsObject())
			{
				Object obj;
				for (auto iter = jsonValue.MemberBegin(); iter != jsonValue.MemberEnd(); iter++)
				{
					const char* key = iter->name.GetString();
					if (!this->WriterToBson(key, obj, iter->value))
					{
						return false;
					}
				}
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, obj);
					return true;
				}
				document.Cast<Array>().Add(obj);
				return true;
			}
			else if (jsonValue.IsArray())
			{
				Bson::Writer::Array arrayDocument;
				for (int index = 0; index < jsonValue.Size(); index++)
				{
					if (!this->WriterToBson(nullptr, arrayDocument, jsonValue[index]))
					{
						return false;
					}
				}
				if (document.IsObject())
				{
					document.Cast<Object>().Add(key, arrayDocument);
					return true;
				}
				document.Cast<Array>().Add(arrayDocument);
				return true;
			}
			return false;
		}

		bool Object::WriterToStream(std::ostream& os)
		{
			const char* str = this->_done();
			const int size = this->len();
			os.write(str, size);
			return true;
		}

		int Object::GetStreamLength()
		{
			this->_done();
			return this->len();
		}

		const char* Object::Serialize(int& length)
		{
			char* bson = this->_done();
			length = this->len();
			return bson;
		}

        void Object::Add(const char *key, Document &document)
        {
            _bson::bsonobjbuilder &build = (_bson::bsonobjbuilder &) document;
            if (document.IsObject())
            {
                _b.appendNum((char) _bson::Object);
            }
            else if (document.IsArray())
            {
                _b.appendNum((char) _bson::Array);
            }
            _b.appendStr(key);
            _b.appendBuf(build._done(), build.len());
        }

		void Object::Add(const char* key, Document& document, BsonDocumentType type)
		{
            _bson::bsonobjbuilder& build = (_bson::bsonobjbuilder&)document;
            switch (type)
            {
                case BsonDocumentType::None:
                    return;
                case BsonDocumentType::Array:
                    _b.appendNum((char)_bson::Array);
                    break;
                case BsonDocumentType::Object:
                    _b.appendNum((char)_bson::Object);
                    break;
            }
			_b.appendStr(key);
			_b.appendBuf(build._done(), build.len());
		}
	}
}
namespace Bson
{
	namespace Read
	{
        bool Object::GetKeys(std::set<std::string> &keys)
        {
            this->getFieldNames(keys);
            return keys.size() > 0;
        }

		bool Object::Get(const char* key, int& value) const
		{
            _bson::bsonelement bsonelement = this->getField(key);
            if (bsonelement.type() == _bson::BSONType::NumberInt)
			{
				value = this->getField(key).Int();
				return true;
			}
			return false;
		}

        bool Object::Get(const char* key, double& value) const
        {
            _bson::bsonelement bsonelement = this->getField(key);
            switch(bsonelement.type())
            {
                case _bson::BSONType::NumberDouble:
                    value = bsonelement.Double();
                    return true;
            }
            return false;
        }

		bool Object::Get(const char* key, bool& value) const
		{
            _bson::bsonelement bsonelement = this->getField(key);
            if (bsonelement.type() == _bson::BSONType::Bool)
			{
				value = bsonelement.Bool();
				return true;
			}
			return false;
		}

		bool Object::Get(const char* key, long long& value) const
		{
            _bson::bsonelement bsonelement = this->getField(key);
            switch(bsonelement.type())
            {
                case _bson::BSONType::NumberInt:
                    value = bsonelement.Int();
                    return true;
                case _bson::BSONType::NumberLong:
                    value = bsonelement.Long();
                    return true;
            }
			return false;
		}

		bool Object::Get(const char* key, std::string& value) const
		{
            _bson::bsonelement bsonelement = this->getField(key);
            if (bsonelement.type() == _bson::BSONType::String)
			{
				value = bsonelement.String();
				return true;
			}
			return false;
		}

		_bson::BSONType Object::Type(const char* key) const
		{
			if (this->hasField(key))
			{
				return this->getField(key).type();
			}
			return _bson::EOO;
		}

		void Object::WriterToJson(std::string& json)
		{
			Json::Writer jsonWriter;
			std::set<std::string> elements;
			this->getFieldNames(elements);
			for (const std::string& key: elements)
			{
				jsonWriter << key;
				this->WriterToJson(this->getField(key), jsonWriter);
			}
			jsonWriter.WriterStream(json);
		}

		void Object::WriterToJson(const _bson::bsonelement& bsonelement, Json::Writer& json)
		{
			switch (bsonelement.type())
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
				json.BeginObject();
				std::set<std::string> elements;
				_bson::bsonobj obj = bsonelement.object();
				obj.getFieldNames(elements);
				for (const std::string& key: elements)
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
				for (const std::string& key: elements)
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
}
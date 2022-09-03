//
// Created by mac on 2022/6/27.
//

#include "BsonDocument.h"

namespace Bson
{
    namespace Writer
    {
        bool Document::FromByJson(const std::string& json)
        {
            rapidjson::Document document;
            if (document.Parse(json.c_str(), json.size()).HasParseError())
            {
                return false;
            }
            if(document.IsArray())
            {
				return false;
            }
            else if(document.IsObject())
            {
                auto iter = document.MemberBegin();
                for (; iter != document.MemberEnd(); iter++)
                {
                    const char *key = iter->name.GetString();
                    if (!this->WriterToBson(key, *this, iter->value))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

    }

	namespace Writer
	{
        void Document::WriterToJson(std::string &json)
        {
            int length = 0;
            const char * bson = this->Serialize(length);
            Bson::Reader::Document obj(bson);
            obj.WriterToJson(json);
        }


		bool Document::WriterToBson(const char* key, Document& document, const rapidjson::Value& jsonValue)
        {
            if (jsonValue.IsString())
            {
                const char *str = jsonValue.GetString();
                const size_t size = jsonValue.GetStringLength();
                document.Add(key, str, size);
                return true;
            }
            else if (jsonValue.IsBool())
            {
                document.Add(key, jsonValue.GetBool());
                return true;
            }
            else if (jsonValue.IsInt())
            {
                document.Add(key, jsonValue.GetInt());
                return true;
            }
            else if (jsonValue.IsInt64())
            {
                long long value = (long long) jsonValue.GetInt64();
                document.Add(key, value);
                return true;
            }
            else if (jsonValue.IsUint())
            {
                document.Add(key, jsonValue.GetUint());
                return true;
            }
            else if (jsonValue.IsUint64())
            {
                document.Add(key, (long long) jsonValue.GetUint64());
                return true;
            }
            else if (jsonValue.IsDouble())
            {
                document.Add(key, jsonValue.GetDouble());
                return true;
            }
            else if (jsonValue.IsFloat())
            {
                document.Add(key, jsonValue.GetFloat());
                return true;
            }
            else if (jsonValue.IsObject())
            {
                Bson::Writer::Document obj;
                for (auto iter = jsonValue.MemberBegin(); iter != jsonValue.MemberEnd(); iter++)
                {
                    const char *key = iter->name.GetString();
                    if (!this->WriterToBson(key, obj, iter->value))
                    {
                        return false;
                    }
                }
                document.Add(key, obj);
                return true;
            }
            else if (jsonValue.IsArray())
            {
                Bson::Writer::Array arrayDocument;
                for (int index = 0; index < jsonValue.Size(); index++)
                {
                    std::string key = std::to_string(index);
                    if (!this->WriterToBson(key.c_str(), arrayDocument, jsonValue[index]))
                    {
                        return false;
                    }
                }
                document.Add(key, arrayDocument);
                return true;
            }
            return false;
        }

		bool Document::WriterToStream(std::ostream& os)
		{
			const char* str = this->_done();
			const int size = this->len();
			os.write(str, size);
			return true;
		}

		int Document::GetStreamLength()
		{
			this->_done();
			return this->len();
		}

		const char* Document::Serialize(int& length)
		{
			char* bson = this->_done();
			length = this->len();
			return bson;
		}


        void Document::Add(const char *key, Document &document)
        {
			_bson::bsonobjbuilder &build = (_bson::bsonobjbuilder &) document;
			switch(document.GetType())
			{
			case DocumentType::Array:
				_b.appendNum((char) _bson::Array);
				break;
			case DocumentType::Object:
				_b.appendNum((char) _bson::Object);
				break;
			}
			_b.appendStr(key);
            _b.appendBuf(build._done(), build.len());
        }
	}
}

namespace Bson
{
	namespace Writer
	{
		void Array::Append(Document& document)
		{
			_bson::bsonobjbuilder &build = (_bson::bsonobjbuilder &) document;

			switch(document.GetType())
			{
			case DocumentType::Array:
				_b.appendNum((char) _bson::Array);
				break;
			case DocumentType::Object:
				_b.appendNum((char) _bson::Object);
				break;
			}
			_b.appendStr(std::to_string(this->mIndex++));
			_b.appendBuf(build._done(), build.len());
		}
	}
}

namespace Bson
{
	namespace Reader
	{
        Document::Document(const char* bson)
                : _bson::bsonobj(bson)
        {

        }

        bool Document::IsOk() const
        {
            double isOk = 0;
            return this->Get("ok", isOk) && isOk != 0;
        }

        bool Document::GetKeys(std::set<std::string> &keys)
        {
            this->getFieldNames(keys);
            return keys.size() > 0;
        }

		bool Document::Get(const char* key, int& value) const
		{
            _bson::bsonelement bsonelement = this->getField(key);
            if (bsonelement.type() == _bson::BSONType::NumberInt)
			{
				value = this->getField(key).Int();
				return true;
			}
			return false;
		}

        bool Document::Get(const char* key, double& value) const
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

		bool Document::Get(const char* key, bool& value) const
		{
            _bson::bsonelement bsonelement = this->getField(key);
            if (bsonelement.type() == _bson::BSONType::Bool)
			{
				value = bsonelement.Bool();
				return true;
			}
			return false;
		}

		bool Document::Get(const char* key, long long& value) const
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

		bool Document::Get(const char* key, std::string& value) const
		{
            _bson::bsonelement bsonelement = this->getField(key);
            if (bsonelement.type() == _bson::BSONType::String)
			{
				value = bsonelement.String();
				return true;
			}
			return false;
		}

		_bson::BSONType Document::Type(const char* key) const
		{
			if (this->hasField(key))
			{
				return this->getField(key).type();
			}
			return _bson::EOO;
		}

		void Document::WriterToJson(std::string& json)
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

		void Document::WriterToJson(const _bson::bsonelement& bsonelement, Json::Writer& json)
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
                const char * bin = bsonelement.binData(len);
				json.AddBinString(bin, len);
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
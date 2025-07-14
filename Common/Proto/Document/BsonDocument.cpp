//
// Created by mac on 2022/6/27.
//

#include"BsonDocument.h"
#include"Proto/Bson/bsonobjiterator.h"


namespace bson
{
	namespace r
	{

		bool Document::GetFirst(bson::r::Document& document) const
		{
			_bson::bsonelement element = this->mObject.firstElement();
			if (element.type() != _bson::BSONType::Object)
			{
				return false;
			}
			document.mObject = element.object();
			return true;
		}
	}
	namespace w
	{
		Document::Document(w::Document& document)
		{
			this->mIndex = 0;
			this->mType = _bson::BSONType::Array;
			this->Push(document);
		}

		Document::Document(bool object)
		{
			this->mIndex = 0;
			this->mType = object ? _bson::BSONType::Object : _bson::BSONType::Array;
		}

		std::string Document::ToString()
		{
			return this->obj().toString();
		}

		bool Document::FromByJson(const json::r::Value& document)
		{
			json::r::Value jsonValue;
			for(const char * key : document.GetAllKey())
			{
				if(document.Get(key, jsonValue))
				{
					if(!this->WriterToBson(key, *this, jsonValue))
					{
						return false;
					}
				}
			}
			return true;
		}

		bool Document::FromByJson(const std::string &json)
		{
			json::r::Document document;
			if (!document.Decode(json, YYJSON_READ_INSITU))
			{
				return false;
			}
			return this->FromByJson(document);
		}

		bool Document::FromByJson(const char* json, size_t size)
		{
			json::r::Document document;
			if (!document.Decode(json, size, YYJSON_READ_INSITU))
			{
				return false;
			}
			return this->FromByJson(document);
		}

		bool Document::WriterToBson(Document& document, const json::r::Value& jsonValue)
		{
			switch(jsonValue.GetType())
			{
				case YYJSON_TYPE_BOOL:
				{
					bool value = false;
					if(jsonValue.Get(value))
					{
						document.Push(value);
					}
					return true;
				}
				case YYJSON_TYPE_NUM:
				{
					float f = 0;
					if(jsonValue.Get(f))
					{
						document.Push(f);
						return true;
					}
					int i = 0;
					if(jsonValue.Get(i))
					{
						document.Push(i);
						return true;
					}
					long long value = 0;
					if(jsonValue.Get(value))
					{
						document.Push(value);
					}
					return true;
				}
				case YYJSON_TYPE_STR:
				{
					std::string value;
					if(jsonValue.Get(value))
					{
						document.Push(value);
					}
					return true;
				}
				case YYJSON_TYPE_OBJ:
				{
					Document document1;
					std::vector<const char *> keys;
					if(jsonValue.GetKeys(keys) > 0)
					{
						json::r::Value value;
						for(const char * key : keys)
						{
							if(jsonValue.Get(key, value))
							{
								if(!this->WriterToBson(key, document1, value))
								{
									return false;
								}
							}
						}
					}
					document.Push(document1);
					return true;
				}
				case YYJSON_TYPE_ARR:
				{
					size_t index = 0;
					json::r::Value value;
					bson::w::Document arrayDocument(_bson::BSONType::Array);
					while(jsonValue.Get(index, value))
					{
						if (!this->WriterToBson(arrayDocument, value))
						{
							return false;
						}
						index++;
					}
					document.Push(arrayDocument);
					return true;
				}
			}
			return false;
		}

		bool Document::WriterToBson(const char *key, Document &document, const json::r::Value &jsonValue)
		{
			switch(jsonValue.GetType())
			{
				case YYJSON_TYPE_STR:
				{
					size_t size = 0;
					const char * str = jsonValue.GetString(size);
					if(str != nullptr)
					{
						document.Add(key, str, size);
					}
					return true;
				}
				case YYJSON_SUBTYPE_REAL:
				{
					float value = 0;
					if(jsonValue.Get(value))
					{
						document.Add(key, value);
					}
					return true;
				}

				case YYJSON_TYPE_NUM:
				{
					long long value = 0;
					if(jsonValue.Get(value))
					{
						if(value >= std::numeric_limits<int>::max())
						{
							document.Add(key, value);
							return true;
						}
						document.Add(key, (int)value);
					}
					return true;
				}
				case YYJSON_TYPE_BOOL:
				{
					bool result = false;
					if(jsonValue.Get(result))
					{
						document.Add(key, result);
					}
					return true;
				}

				case YYJSON_TYPE_OBJ:
				{
					json::r::Value value;
					long long longNumber = 0;
					std::vector<const char *> keys;
					bson::w::Document document1;
					if(jsonValue.Get("int64", longNumber))
					{
						document.Add(key, longNumber);
						return true;
					}
					else if(jsonValue.GetKeys(keys) > 0)
					{
						for(const char * k : keys)
						{
							if(jsonValue.Get(k, value))
							{
								if(!this->WriterToBson(k, document1, value))
								{
									return false;
								}
							}
						}
					}
					document.Add(key, document1);
					return true;
				}
				case YYJSON_TYPE_ARR:
				{
					size_t index = 0;
					json::r::Value value;
					bson::w::Document arrayDocument(_bson::BSONType::Array);
					while(jsonValue.Get(index, value))
					{
						if (!this->WriterToBson(arrayDocument, value))
						{
							return false;
						}
						index++;
					}
					document.Add(key, arrayDocument);
					return true;
				}
				case YYJSON_TYPE_NULL:
				{
					document.Add(key);
					return true;
				}
			}
			return false;
		}

		const char *Document::Serialize(int &length)
		{
			char *bson = this->_done();
			length = this->len();
			return bson;
		}

		bool Document::Add(const char* key)
		{
			if(this->mType != _bson::BSONType::Object)
			{
				return false;
			}
			this->appendNull(key);
			return true;
		}

		bool Document::Add(const char *key, Document &document)
		{
			if(this->mType != _bson::BSONType::Object)
			{
				return false;
			}
			this->bb().appendNum((char)document.mType);
			this->bb().appendStr(key);

			int len = 0;
			const char * str = document.Serialize(len);
			this->bb().appendBuf(str, len);
			return true;
		}

		bool Document::Add(const char* key, const json::r::Value& document)
		{
			bson::w::Document document1;
			if(!document1.FromByJson(document))
			{
				return false;
			}
			this->Add(key, document1);
			return true;
		}

		bool Document::AddObject(const char* key, const std::string & json)
		{
			bson::w::Document document;
			if(!document.FromByJson(json))
			{
				return false;
			}
			return this->Add(key, document);
		}

		bool Document::Add(const char* key, r::Value& document)
		{
			if(this->mType != _bson::BSONType::Object)
			{
				return false;
			}
			this->bb().appendNum((char)document.Type());
			this->bb().appendStr(key);
			this->appendAs(document.ToElement(), key);
			return true;
		}

		bool Document::Add(const char* key, r::Document& document)
		{
			if(this->mType != _bson::BSONType::Object)
			{
				return false;
			}
			this->append(key, document.ToObject());
			return true;
		}
	}
}
namespace bson
{
	namespace r
    {
        bool Document::WriterToJson(std::string * jsonStr)
        {
            json::w::Document jsonWriter;
			this->WriterToJson(jsonWriter);
			return jsonWriter.Serialize(jsonStr);
        }

		bool Document::WriterToJson(json::w::Document& document)
		{
			_bson::bsonobjiterator bsonIterator(this->mObject);
			while(bsonIterator.more())
			{
				_bson::bsonelement bsonElement = bsonIterator.next();
				this->WriterToJson(bsonElement, document);
			}
			return true;
		}

        void Document::WriterToJson(const _bson::bsonelement &element, json::w::Value &json)
        {
            switch (element.type())
            {
                case _bson::BSONType::Bool:
					if(json.IsArray())
					{
						json.Push(element.Bool());
						return;
					}
					json.Add(element.fieldName(), element.Bool());
                    break;
                case _bson::BSONType::String:
					if(json.IsArray())
					{
						json.Push(element.String());
						return;
					}
                    json.Add(element.fieldName(), element.String());
                    break;
                case _bson::BSONType::NumberInt:
					if(json.IsArray())
					{
						json.Push(element.Int());
						return;
					}
                    json.Add(element.fieldName(), element.Int());
                    break;
                case _bson::BSONType::NumberLong:
					if(json.IsArray())
					{
						json.Push(element.Long());
						return;
					}
                    json.Add(element.fieldName(), element.Long());
                    break;
                case _bson::BSONType::NumberDouble:
					if(json.IsArray())
					{
						json.Push(element.Double());
						return;
					}
                    json.Add(element.fieldName(), element.Double());
                    break;
                case _bson::BSONType::BinData:
                {
                    int len = 0;
                    const char *str = element.binData(len);
					if(json.IsArray())
					{
						json.Push(str, len);
						return;
					}
                    json.Add(element.fieldName(), str, len);
					break;
				}
                case _bson::BSONType::Object:
                {
					std::unique_ptr<json::w::Value> data =
							json.IsArray() ? json.AddObject() : json.AddObject(element.fieldName());
					{
						std::set<std::string> elements;
						_bson::bsonobj obj = element.object();
						obj.getFieldNames(elements);
						for (const std::string &key: elements)
						{
							this->WriterToJson(obj.getField(key), *data);
						}
					}
					break;
				}
                case _bson::BSONType::Array:
                {
					std::unique_ptr<json::w::Value> data = json.IsArray() ?
							json.AddArray() : json.AddArray(element.fieldName());
					{
						std::vector<_bson::bsonelement> elements = element.Array();
						for (const _bson::bsonelement & bsonelement: elements)
						{
							this->WriterToJson(bsonelement, *data);
						}
					}
					break;
				}
                case _bson::BSONType::Timestamp:
					if(json.IsArray())
					{
						json.Push((long long)element.timestampValue());
						return;
					}
                    json.Add(element.fieldName(), (long long)element.timestampValue());
                    break;
                case _bson::BSONType::Date:
					if(json.IsArray())
					{
						json.Push((long long)element.date().asInt64());
						return;
					}
                    json.Add(element.fieldName(), (long long) element.date().asInt64());
                    break;
				case _bson::BSONType::jstOID:
					if(json.IsArray())
					{
						json.Push(element.__oid().toString());
						return;
					}
					json.Add(element.fieldName(), element.__oid().toString());
					break;
                default:
					if(json.IsArray())
					{
						json.Push(element.toString());
						return;
					}
                    json.Add(element.fieldName(), element.toString());
                    break;
            }
        }
	}
}
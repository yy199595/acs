//
// Created by mac on 2022/6/27.
//

#include"BsonDocument.h"
#include"Proto/Bson/bsonobjiterator.h"
namespace bson
{
	namespace Writer
	{
		void Array::Add(Array &document)
		{
			_bson::bsonobjbuilder &build = (_bson::bsonobjbuilder &)document;
			_b.appendNum((char)_bson::Array);
			_b.appendStr(std::to_string(this->mIndex++));

			const char * str = build._done();

			_b.appendBuf(str, build.len());
		}

		void Array::Add(Document &document)
		{
			_bson::bsonobjbuilder &build = (_bson::bsonobjbuilder &)document;

			_b.appendNum((char)_bson::Object);
			_b.appendStr(std::to_string(this->mIndex++));

			const char * str = build._done();
			_b.appendBuf(str, build.len());
		}
	}
}

namespace bson
{
	namespace Reader
	{
		Document::Document(const _bson::bsonobj& object)
			: mObject(object)
		{

		}

		bool Document::IsOk() const
		{
			double isOk = 0;
			return this->Get("ok", isOk) && isOk != 0;
		}

		bool Document::Get(const char *key, double &value) const
        {
            _bson::bsonelement element = this->mObject.getField(key);
            if (element.type() != _bson::BSONType::NumberDouble)
            {
                return false;
            }
            value = element.Double();
            return true;
        }


		bool Document::Get(const char* key, std::vector<std::string>& document) const
		{
			_bson::bsonelement element = this->mObject.getField(key);
			if (element.type() != _bson::BSONType::Array)
			{
				return false;
			}

			for(const _bson::bsonelement & element1 : element.Array())
			{
				switch(element1.type())
				{
					case _bson::BSONType::String:
						document.emplace_back(element1.String());
						break;
					case _bson::BSONType::BinData:
					{
						int len = 0;
						const char* bin = element1.binData(len);
						document.emplace_back(bin, len);
						break;
					}
					case _bson::BSONType::Object:
					{
						std::string json;
						bson::Reader::Document doc(element1.object());
						doc.WriterToJson(&json);
						document.emplace_back(json);
						break;
					}
					default:
						return false;
				}
			}
			return true;
		}

		bool Document::Get(const char* key, std::vector<_bson::bsonelement>& document) const
		{
			_bson::bsonelement element = this->mObject.getField(key);
			if (element.type() != _bson::BSONType::Object)
			{
				return false;
			}
			document = element.Array();
			return true;
		}

		bool Document::Get(const char* key, std::vector<std::unique_ptr<Document>>& document) const
		{
			_bson::bsonelement element = this->mObject.getField(key);
			if(element.type() != _bson::BSONType::Array)
			{
				return false;
			}
			std::vector<_bson::bsonelement> arr = element.Array();
			for(_bson::bsonelement & item : arr)
			{
				if (item.type() != _bson::BSONType::Object)
				{
					return false;
				}
				document.emplace_back(std::make_unique<Document>(item.object()));
			}
			return true;
		}


		bool Document::Get(const char* key, std::unique_ptr<Document>& document) const
		{
			_bson::bsonelement element = this->mObject.getField(key);
			if (element.type() != _bson::BSONType::Object)
			{
				return false;
			}
			document = std::make_unique<Document>(element.object());
			return true;
		}
	}
	namespace Writer
	{
		std::string Document::ToString()
		{
			std::string json;
			bson::Reader::Document obj(this->obj());
			obj.WriterToJson(&json);
			return json;
		}

		bool Document::FromByJson(const json::r::Document& document)
		{
			std::vector<const char *> keys;
			if(document.GetKeys(keys) < 0)
			{
				return false;
			}
			std::unique_ptr<json::r::Value> jsonValue;
			for(const char * key : keys)
			{
				if(document.Get(key, jsonValue))
				{
					if(!this->WriterToBson(key, *this, *jsonValue))
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
			if (!document.Decode(json))
			{
				return false;
			}
			return this->FromByJson(document);
		}

		bool Document::WriterToBson(Array& document, const json::r::Value& jsonValue)
		{
			switch(jsonValue.GetType())
			{
				case YYJSON_TYPE_BOOL:
				{
					bool value = false;
					if(jsonValue.Get(value))
					{
						document.Add(value);
					}
					return true;
				}
				case YYJSON_TYPE_NUM:
				{
					float f = 0;
					if(jsonValue.Get(f))
					{
						document.Add(f);
						return true;
					}
					int i = 0;
					if(jsonValue.Get(i))
					{
						document.Add(i);
						return true;
					}
					long long value = 0;
					if(jsonValue.Get(value))
					{
						document.Add(value);
					}
					return true;
				}
				case YYJSON_TYPE_STR:
				{
					std::string value;
					if(jsonValue.Get(value))
					{
						document.Add(value);
					}
					return true;
				}
				case YYJSON_TYPE_OBJ:
				{
					Document document1;
					std::vector<const char *> keys;
					if(jsonValue.GetKeys(keys) > 0)
					{
						std::unique_ptr<json::r::Value> value;
						for(const char * key : keys)
						{
							if(jsonValue.Get(key, value))
							{
								if(!this->WriterToBson(key, document1, *value))
								{
									return false;
								}
							}
						}
					}
					document.Add(document1);
					return true;
				}
				case YYJSON_TYPE_ARR:
				{
					size_t index = 0;
					bson::Writer::Array arrayDocument;
					std::unique_ptr<json::r::Value> value;
					while(jsonValue.Get(index, value))
					{
						if (!this->WriterToBson(arrayDocument, *value))
						{
							return false;
						}
						index++;
					}
					document.Add(arrayDocument);
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
					std::string result;
					if(jsonValue.Get(result))
					{
						document.Add(key, result);
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
					long long longNumber = 0;
					std::vector<const char *> keys;
					bson::Writer::Document document1;
					std::unique_ptr<json::r::Value> value;
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
								if(!this->WriterToBson(k, document1, *value))
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
					bson::Writer::Array arrayDocument;
					std::unique_ptr<json::r::Value> value;
					while(jsonValue.Get(index, value))
					{
						if (!this->WriterToBson(arrayDocument, *value))
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

		void Document::Add(const char *key, Array &document)
		{
			_bson::bsonobjbuilder &build = (_bson::bsonobjbuilder &)document;

			this->bb().appendNum((char)_bson::Array);
			this->bb().appendStr(key);

			const char * str = build._done();
			this->bb().appendBuf(str, build.len());
		}
		

		void Document::Add(const char* key)
		{
			this->appendNull(key);
		}

		void Document::Add(const char *key, Document &document)
		{
			_bson::bsonobjbuilder &build = (_bson::bsonobjbuilder &)document;

			this->bb().appendNum((char)_bson::Object);
			this->bb().appendStr(key);
			const char * str = build._done();
			this->bb().appendBuf(str, build.len());
		}
	}
}
namespace bson
{
	namespace Reader
    {
        bool Document::Get(const char *key, int &value) const
        {
            _bson::bsonelement element = this->mObject.getField(key);
            if (element.type() == _bson::BSONType::NumberInt)
            {
                value = element.Int();
                return true;
            }
            return false;
        }

        bool Document::Get(const char *key, bool &value) const
        {
            _bson::bsonelement element = this->mObject.getField(key);
            if (element.type() == _bson::BSONType::Bool)
            {
                value = element.Bool();
                return true;
            }
            return false;
        }

        bool Document::Get(const char *key, long long &value) const
        {
            _bson::bsonelement element = this->mObject.getField(key);
            switch (element.type())
            {
                case _bson::BSONType::NumberInt:
                    value = element.Int();
                    return true;
                case _bson::BSONType::NumberLong:
                    value = element.Long();
                    return true;
                default:
                    return false;
            }
        }

        bool Document::Get(const char *key, std::string &value) const
        {
            _bson::bsonelement element = this->mObject.getField(key);
            switch (element.type())
            {
                case _bson::BSONType::String:
                    value = element.String();
                    return true;
                case _bson::BSONType::BinData:
                {
                    int len = 0;
                    const char *str = element.binData(len);
                    value.append(str, len);
                    return true;
                }
                default:
                    return false;
            }
        }

        _bson::BSONType Document::Type(const char *key) const
        {
            _bson::bsonelement element = this->mObject.getField(key);
            return element.type();
        }

        bool Document::WriterToJson(std::string * jsonStr)
        {
            json::w::Document jsonWriter;
            _bson::bsonobjiterator bsonIterator(this->mObject);
			while(bsonIterator.more())
			{
				_bson::bsonelement bsonElement = bsonIterator.next();
				this->WriterToJson(bsonElement, jsonWriter);
			}
            return jsonWriter.Encode(jsonStr);
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
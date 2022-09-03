//
// Created by mac on 2022/6/27.
//

#ifndef SERVER_BSONDOCUMENT_H
#define SERVER_BSONDOCUMENT_H

#include"Bson/bsonobj.h"
#include"Json/JsonWriter.h"
#include"rapidjson/document.h"
#include"Bson/bsonobjbuilder.h"
namespace Bson
{
    enum class DocumentType
    {
        None,
        Array,
        Object,
    };
	namespace Writer
	{
		class Document : protected _bson::bsonobjbuilder
		{
		public:
            Document() = default;
			~Document() = default;

		public:
            void WriterToJson(std::string & json);
            bool FromByJson(const std::string& json);
            const std::string & GetId() const { return this->mId; }
		public:
			const char* Serialize(int& length);

			void Add(Document& document);
		 public:
			void Add(const char* key, Document& document);

			inline void Add(const char* key, int value)
			{
				this->append(key, value);
			}

			inline void Add(const char* key, bool value)
			{
                this->append(key, value);
			}

			inline void Add(const char* key, double value)
			{
                this->append(key, value);
			}

			inline void Add(const char* key, long long value)
			{
                this->append(key, value);
			}

			inline void Add(const char* key, const char* value)
			{
                this->append(key, value);
			}

			inline void Add(const char* key, unsigned int value)
			{
                this->append(key, value);
			}

			inline void Add(const char* key, const std::string& value)
			{
                this->append(key, value);
			}

			inline void Add(const char* key, const char* str, size_t size)
			{
                this->append(key, str, size + 1);
			}

        public:
			int GetStreamLength();
			bool WriterToStream(std::ostream& os);
            virtual DocumentType GetType() const { return DocumentType::Object; }
			bool WriterToBson(const char* key, Document& document, const rapidjson::Value& jsonValue);
        private:
            std::string mId;
		};
	}

	namespace Writer
	{
		class Array : public Document
		{
		 public:
			Array() : mIndex(0) { }
			void Append(Document & document);
			virtual DocumentType GetType() const { return DocumentType::Array; }
		 private:
			int mIndex;
		};
	}

	namespace Reader
	{
		class Document : protected _bson::bsonobj
		{
		public:
            Document(const char* bson);

		public:
			void WriterToJson(std::string& json);

			_bson::BSONType Type(const char* key) const;

			bool IsOk() const;

			bool Get(const char* key, int& value) const;

			bool Get(const char* key, bool& value) const;

			bool Get(const char* key, double& value) const;

			bool Get(const char* key, long long& value) const;

			bool Get(const char* key, std::string& value) const;

            bool GetKeys(std::set<std::string> & keys);
            
            int Length() const { return this->objsize();}
		private:
			void WriterToJson(const _bson::bsonelement& bsonelement, Json::Writer& json);
		};
	}
}


#endif //SERVER_BSONDOCUMENT_H

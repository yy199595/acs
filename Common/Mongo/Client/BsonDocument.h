//
// Created by mac on 2022/6/27.
//

#ifndef SERVER_BSONDOCUMENT_H
#define SERVER_BSONDOCUMENT_H

#include"Mongo/Bson/bsonobj.h"
#include"rapidjson/document.h"
#include"Util/Json/JsonWriter.h"
#include"Mongo/Bson/bsonobjbuilder.h"
namespace Bson
{

	namespace Writer
	{
		class Document;
		class WriterDocument;

		class Array : public _bson::bsonobjbuilder
		{
		public:
			Array() : mIndex(0) { }
            Array(Document & document) : mIndex(0) { this->Add(document); }
		public:
			void Add(int value);

			void Add(bool value);

			void Add(double value);

			void Add(long long value);

			void Add(unsigned int value);

			void Add(Array& document);

			void Add(Document& document);

			void Add(const std::string& value);

			void Add(const char* str, size_t size);

		private:
			int mIndex;
		};
	}

	namespace Writer
	{
		class Document : public _bson::bsonobjbuilder
		{
		public:
			Document() = default;
			~Document() = default;

		public:
            void WriterToJson(std::string & json);
            bool FromByJson(const std::string& json);
        public:
			const char* Serialize(int& length);

			void Add(const char* key, Array& document);
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

			std::string ToJson();

			bool WriterToStream(std::ostream& os);

			bool WriterToBson(Array& document, const rapidjson::Value& jsonValue);

			bool WriterToBson(const char* key, Document& document, const rapidjson::Value& jsonValue);
		};
	}

	namespace Reader
	{
		class Document
		{
		public:
			Document(const char* bson);
			Document(_bson::bsonobj object);
		public:
			void WriterToJson(std::string * json);

			_bson::BSONType Type(const char* key) const;

			bool IsOk() const;

			bool Get(const char* key, int& value) const;

			bool Get(const char* key, bool& value) const;

			bool Get(const char* key, double& value) const;

			bool Get(const char* key, long long& value) const;

			bool Get(const char* key, std::string& value) const;

			int Length() const { return this->mObject.objsize(); }

			bool Get(const char * key, std::shared_ptr<Document> & document);

			bool Get(const char * key, std::vector<std::shared_ptr<Document>> & document);

			bool Get(const char * key, std::vector<std::string> & document);

			bool Get(const char * key, std::vector<_bson::bsonelement> & document);

			int GetLength() const { return this->mObject.objsize(); }
		private:
			void WriterToJson(const _bson::bsonelement& bsonelement, Json::Writer& json);
		private:
			_bson::bsonobj mObject;
		};
	}
}


#endif //SERVER_BSONDOCUMENT_H

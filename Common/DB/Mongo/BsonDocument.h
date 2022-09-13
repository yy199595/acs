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

		public:
            void WriterToJson(std::string & json);
            bool FromByJson(const std::string& json);
            bool FromByJson(const std::string& json, std::string & id);
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

			bool WriterToStream(std::ostream& os);

			bool WriterToBson(Array& document, const rapidjson::Value& jsonValue);

			bool WriterToBson(const char* key, Document& document, const rapidjson::Value& jsonValue);
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

            int Length() const { return this->objsize();}
		private:
			void WriterToJson(const _bson::bsonelement& bsonelement, Json::Writer& json);
		};
	}
}


#endif //SERVER_BSONDOCUMENT_H

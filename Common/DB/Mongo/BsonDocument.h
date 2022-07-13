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
		class WriterDocument;

		class Document : protected _bson::bsonobjbuilder
		{
		public:
			virtual bool IsArray() = 0;
			virtual bool IsObject() = 0;
		public:
			template<typename T>
			T & Cast() { return (T&)(*this);}
		};

		class Array : public Document
		{
		public:
			Array() : mIndex(0) { }
            Array(Document & document) : mIndex(0) { this->Add(document); }
		public:
			bool IsArray() final
			{
				return true;
			}

			bool IsObject() final
			{
				return false;
			}

		public:
			void Add(int value);

			void Add(bool value);

			void Add(double value);

			void Add(long long value);

			void Add(unsigned int value);

			void Add(Document& document);

			void Add(const std::string& value);

			void Add(const char* str, size_t size);

		private:
			int mIndex;
		};
	}

	namespace Writer
	{
		class Object : public Document
		{
		public:
			Object() = default;

			~Object() = default;

		public:
			bool IsArray() final
			{
				return false;
			}

			bool IsObject() final
			{
				return true;
			}

		public:
			bool FromByJson(const std::string& json);

		public:
			const char* Serialize(int& length);

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

			bool WriterToBson(const char* key, Document& document, const rapidjson::Value& jsonValue);
		};
	}

	namespace Read
	{
		class Object : protected _bson::bsonobj
		{
		public:
			Object(const char* bson);

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



#ifndef APP_BSONDOCUMENT_H
#define APP_BSONDOCUMENT_H

#include"Proto/Bson/bsonobj.h"
#include"Yyjson/Document/Document.h"
#include"Proto/Bson/bsonobjbuilder.h"
#include"Proto/Bson/bsonobjiterator.h"
namespace bson
{

	namespace Writer
	{
		class Document;
		class Array : protected _bson::bsonobjbuilder
		{
		public:
			Array() : mIndex(0) { }
            explicit Array(Document & document) : mIndex(0) { this->Add(document); }
		public:
			template<typename T>
			inline void Add(const T & value) {
				int index = this->mIndex++;
				this->append(std::to_string(index), value);
			}

			void Add(Array& document);

			void Add(Document& document);
		private:
			int mIndex;
		};
	}

	namespace Writer
	{
		class Document : protected _bson::bsonobjbuilder
		{
		public:
			Document() = default;
			~Document() = default;
		public:
			std::string ToString();
            bool FromByJson(const std::string& json);
			bool FromByJson(const json::r::Document & json);
		public:
			const char* Serialize(int& length);

			void Add(const char * key);
			void Add(const char* key, Array& document);
			void Add(const char* key, Document& document);

			template<typename T>
			inline void Add(const char * key, const T & value)
			{
				this->append(key, value);
			}

			inline void Add(const char* key, const char* str, size_t size)
			{
				this->append(key, str, size + 1);
			}
		private:
			bool WriterToBson(Array& document, const json::r::Value& jsonValue);
			bool WriterToBson(const char* key, Document& document, const json::r::Value& jsonValue);
		private:
		};
	}

	namespace Reader
	{
		class Document
		{
		public:
			explicit Document() = default;
			explicit Document(const _bson::bsonobj& object);
		public:


			bool WriterToJson(std::string * json);

			_bson::BSONType Type(const char* key) const;

			bool IsOk() const;

			bool Get(const char* key, int& value) const;

			bool Get(const char* key, bool& value) const;

			bool Get(const char* key, double& value) const;

			bool Get(const char* key, long long& value) const;

			bool Get(const char* key, std::string& value) const;

			bool Get(const char * key, std::unique_ptr<Document> & document) const;

			bool Get(const char * key, std::vector<std::unique_ptr<Document>> & document) const;

			bool Get(const char * key, std::vector<std::string> & document) const;

			bool Get(const char * key, std::vector<_bson::bsonelement> & document) const;

			inline std::string ToString()
			{
				std::string result;
				this->WriterToJson(&result);
				return result;
			}
			inline void Init(const char * bson) { this->mObject.init(bson); }
			inline std::string ToBson() const { return this->mObject.toString(); }
		private:
			void WriterToJson(const _bson::bsonelement& bsonelement, json::w::Value& json);
		private:
			_bson::bsonobj mObject;
		};
	}
}


#endif //APP_BSONDOCUMENT_H

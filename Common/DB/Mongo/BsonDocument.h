//
// Created by mac on 2022/6/27.
//

#ifndef SERVER_BSONDOCUMENT_H
#define SERVER_BSONDOCUMENT_H

#include"Bson/bsonobj.h"
#include"Json/JsonWriter.h"
#include"Bson/bsonobjbuilder.h"
namespace Bson
{
	class WriterDocument;
	class ArrayDocument : protected _bson::bsonobjbuilder
	{
	 public:
		ArrayDocument() : mIndex(0) {}
	 public:
		void Add(int value);
		void Add(bool value);
		void Add(long long value);
		void Add(WriterDocument & document);
		void Add(const std::string & value);
		void Add(const char * str, size_t size);
	 private:
		int mIndex;
	};

	class WriterDocument : protected _bson::bsonobjbuilder
	{
	public:
		WriterDocument() = default;
		~WriterDocument() = default;
	public:
		const char * Serialize(int & length);
		void Add(const char * key, ArrayDocument & document);
		void Add(const char * key, WriterDocument & document);
		inline void Add(const char* key, int value) { this->append(key, value);}
		inline void Add(const char* key, bool value) { this->append(key, value);}
		inline void Add(const char* key, double value){ this->append(key, value);}
		inline void Add(const char* key, long long value){ this->append(key, value);}
		inline void Add(const char* key, const char * value){ this->append(key, value);}
		inline void Add(const char* key, const std::string & value){ this->append(key, value);}
	 public:
		int GetStreamLength();
		bool WriterToStream(std::ostream& os);
	};

	class ReaderDocument : protected _bson::bsonobj
	{
	public:
		ReaderDocument(const char * bson);
	public:
		void WriterToJson(std::string & json);

		_bson::BSONType Type(const char * key) const;

		bool IsOk() const;

		bool Get(const char* key, int& value) const;

		bool Get(const char* key, bool& value) const;

		bool Get(const char* key, double & value) const;

		bool Get(const char* key, long long& value) const;

		bool Get(const char* key, std::string& value) const;

		const std::string ToString() const { return this->toString(); }
	private:
		void WriterToJson(const _bson::bsonelement & bsonelement, Json::Writer & json);
	};
}


#endif //SERVER_BSONDOCUMENT_H

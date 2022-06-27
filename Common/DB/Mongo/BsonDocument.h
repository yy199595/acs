//
// Created by mac on 2022/6/27.
//

#ifndef SERVER_BSONDOCUMENT_H
#define SERVER_BSONDOCUMENT_H
#include"Bson/bsonobj.h"
#include"Bson/bsonobjbuilder.h"

namespace Bson
{
	class WriterDocument
	{
	public:
		WriterDocument() = default;
		~WriterDocument() = default;
	public:
		bool Add(const char* key, int value);
		bool Add(const char* key, bool value);
		bool Add(const char* key, long long value);
		bool Add(const char* key, const std::string & value);

	public:
		bool WriterToStream(std::ostream& os);

		int GetStreamLength() const { return this->mBsonBuilder.len(); }

	private:
		_bson::bsonobjbuilder mBsonBuilder;
	};

	class ReaderDocument
	{
	public:
		ReaderDocument() = default;
		~ReaderDocument() { delete this->mObject; }
	public:
	public:
		bool Get(const char* key, int& value);

		bool Get(const char* key, bool& value);

		bool Get(const char* key, long long& value);

		bool Get(const char* key, std::string& value);

	public:
		void WriterToJson(std::string & json);
		bool ParseFromStream(std::istream& is);
	private:
		std::string mBuffer;
		_bson::bsonobj* mObject;
	};
}


#endif //SERVER_BSONDOCUMENT_H

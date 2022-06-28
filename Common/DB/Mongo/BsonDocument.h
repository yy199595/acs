//
// Created by mac on 2022/6/27.
//

#ifndef SERVER_BSONDOCUMENT_H
#define SERVER_BSONDOCUMENT_H

#include"Bson/bsonobj.h"
#include"Bson/bsonobjbuilder.h"
namespace Bson
{
	class WriterDocument : protected _bson::bsonobjbuilder
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
		int GetStreamLength();
		bool WriterToStream(std::ostream& os);
	};

	class ReaderDocument : protected _bson::bsonobj
	{
	public:
		ReaderDocument(const char * bson);
	public:
		void WriterToJson(std::string & json);

		bool Get(const char* key, int& value) const;

		bool Get(const char* key, bool& value) const;

		bool Get(const char* key, long long& value) const;

		bool Get(const char* key, std::string& value) const;
	};
}


#endif //SERVER_BSONDOCUMENT_H

//
// Created by mac on 2022/6/27.
//

#ifndef SERVER_BSONDOCUMENT_H
#define SERVER_BSONDOCUMENT_H
#include"Bson/BsonObject.h"
#include"Bson/BsonObjectBuilder.h"

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
		int GetStreamLength();

		bool WriterToStream(std::ostream& os);


	private:
		Bson::BsonObjectBuilder mBsonBuilder;
	};

	class ReaderDocument
	{
	public:
		ReaderDocument() = default;
		~ReaderDocument() { delete this->mObject; }
	public:
	public:
		bool Get(const char* key, int& value) const;

		bool Get(const char* key, bool& value) const;

		bool Get(const char* key, long long& value) const;

		bool Get(const char* key, std::string& value) const;

	public:
		void WriterToJson(std::string & json);
		bool ParseFromStream(std::istream& is);
	private:
		std::string mBuffer;
		Bson::BsonObject* mObject;
	};
}


#endif //SERVER_BSONDOCUMENT_H

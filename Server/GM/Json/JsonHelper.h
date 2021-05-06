#pragma once
#include<vector>
#include<rapidjson/writer.h>
#include<rapidjson/document.h>
#include<rapidjson/stringbuffer.h>
using namespace std;
namespace SoEasy
{
	class RapidJsonWriter
	{
	public:
		RapidJsonWriter() :jsonWriter(nullptr) { this->StartAddParameter(); }
		const std::string & Serialization();
		bool AddParameter(const char * key, const int value);
		bool AddParameter(const char * key, const double value);
		bool AddParameter(const char * key, const char * value);
		bool AddParameter(const char * key, const long long value);
		bool AddParameter(const char * key, const std::string value);
		bool AddParameter(const char * key, const unsigned int value);
		bool AddParameter(const char * key, const unsigned long long value);
		bool AddParameter(const char * key, const std::vector<std::string> value);
	public:
		bool StartAddParameter();
	private:
		std::string mTempString;
		rapidjson::StringBuffer strBuf;
		rapidjson::Writer<rapidjson::StringBuffer> * jsonWriter;
	};
}

namespace SoEasy
{
	class RapidJsonReader
	{
	public:
		bool TryParse(const std::string & str);
		bool TryParse(const char * str, const size_t size);
	public:
		bool TryGetValue(const char * key, int & data);
		bool TryGetValue(const char * key, bool & data);
		bool TryGetValue(const char * key, short & data);
		bool TryGetValue(const char * key, float & data);
		bool TryGetValue(const char * key, double & data);
		bool TryGetValue(const char * key, long long & data);
		bool TryGetValue(const char * key, std::string & data);
		bool TryGetValue(const char * key, unsigned int & data);
		bool TryGetValue(const char * key, unsigned short & data);
		bool TryGetValue(const char * key, unsigned long long & data);
		bool TryGetValue(const char * key, std::vector<std::string> & data);
		std::string & GetJsonString() { return this->mJsonString; }
	private:
		std::string mJsonString;
		rapidjson::Document document;
		typedef rapidjson::Document::MemberIterator MemberIter;
	};
}
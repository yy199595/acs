//
// Created by yjz on 2022/3/26.
//

#ifndef _JSONWRITER_H_
#define _JSONWRITER_H_
#include<list>
#include<string>
#include<vector>
#include<ostream>
#include"XCode/XCode.h"
#include<rapidjson/writer.h>
#include<rapidjson/document.h>
#include<rapidjson/stringbuffer.h>
namespace Json
{
	enum class JsonType
	{
		StartObject = 1,
		StartArray = 2,
		EndObject = 3,
		EndArray = 4
	};

	class Writer : protected rapidjson::Writer<rapidjson::StringBuffer>
	{
	 public:
		Writer(bool isObj = true);
	public:
		Writer& operator <<(JsonType type);
		Writer & operator << (std::vector<int> & value);
		Writer & operator << (std::list<std::string> & value);
		Writer & operator << (std::vector<std::string> & value);
		inline Writer& operator <<(int value) { this->Int(value); return *this;}
		inline Writer& operator <<(bool value) { this->Bool(value); return *this;}
		inline Writer& operator <<(float value) { this->Double(value); return *this;}
		inline Writer& operator <<(double value) { this->Double(value); return *this;}
		inline Writer& operator <<(unsigned int value) { this->Uint(value); return *this;}
		inline Writer& operator <<(long long value) { this->Int64(value); return *this;}
		inline Writer& operator <<(const char * value) { this->String(value); return *this;}
		inline Writer& operator <<(const std::string & value) { this->String(value.c_str(), value.size()); return *this;}
	 public:
		size_t GetJsonSize();
		const std::string ToJsonString();
		size_t WriterStream(std::string& os);
		size_t WriterStream(std::ostream& os);
		void AddBinString(const char * str, size_t size);
		bool GetDocument(rapidjson::Document & jsonDocument);
	 private:
		const bool mIsObject;
		rapidjson::StringBuffer mStringBuf;
	};
}
#endif //_JSONWRITER_H_

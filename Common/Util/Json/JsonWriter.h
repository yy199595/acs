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
#include"rapidjson/writer.h"
#include"rapidjson/document.h"
#include"rapidjson/stringbuffer.h"
namespace Json
{
	enum class End
	{
		EndArray,
		EndObject,
	};

	class Writer
	{
	 public:
		Writer(bool isObj = true);
        ~Writer();
	public:
		Writer & BeginArray();
		Writer & BeginObject();
		Writer & BeginArray(const char * key);
		Writer & BeginObject(const char * key);

		Writer& operator <<(Json::End type);
		Writer & operator << (std::vector<int> & value);
		Writer & operator << (std::list<std::string> & value);
		Writer & operator << (std::vector<std::string> & value);
		inline Writer& operator <<(int value) { this->mWriter->Int(value); return *this;}
		inline Writer& operator <<(bool value) { this->mWriter->Bool(value); return *this;}
		inline Writer& operator <<(float value) { this->mWriter->Double(value); return *this;}
		inline Writer& operator <<(double value) { this->mWriter->Double(value); return *this;}
		inline Writer& operator <<(unsigned int value) { this->mWriter->Uint(value); return *this;}
		inline Writer& operator <<(long long value) { this->mWriter->Int64(value); return *this;}
		inline Writer& operator <<(const char * value) { this->mWriter->String(value); return *this;}
		inline Writer& operator <<(unsigned long long value) { this->mWriter->Uint64(value); return *this;}
		inline Writer& operator <<(const std::string & value) { this->mWriter->String(value.c_str(), value.size()); return *this;}
	 public:
		size_t GetJsonSize();
		const std::string JsonString();
		size_t WriterStream(std::string& os);
		size_t WriterStream(std::ostream& os);
		void AddBinString(const char * str, size_t size);
		bool GetDocument(rapidjson::Document & jsonDocument);
	 private:
		const bool mIsObject;
		rapidjson::StringBuffer * mStringBuf;
        rapidjson::Writer<rapidjson::StringBuffer> * mWriter;
	};
}
#endif //_JSONWRITER_H_

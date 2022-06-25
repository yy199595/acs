//
// Created by yjz on 2022/3/26.
//

#ifndef _JSONWRITER_H_
#define _JSONWRITER_H_
#include<string>
#include<vector>
#include<ostream>
#include"XCode/XCode.h"
#include<rapidjson/writer.h>
#include<rapidjson/document.h>
#include<rapidjson/stringbuffer.h>
namespace Json
{
	class Writer
	{
	 public:
		Writer(bool isObj = true);

	public:
		Writer & operator << (std::vector<int> & value);
		Writer & operator << (std::vector<std::string> & value);
		inline Writer& operator <<(int value) { this->mJsonWriter.Int(value); return *this;}
		inline Writer& operator <<(bool value) { this->mJsonWriter.Bool(value); return *this;}
		inline Writer& operator <<(float value) { this->mJsonWriter.Double(value); return *this;}
		inline Writer& operator <<(double value) { this->mJsonWriter.Double(value); return *this;}
		inline Writer& operator <<(unsigned int value) { this->mJsonWriter.Uint(value); return *this;}
		inline Writer& operator <<(long long value) { this->mJsonWriter.Int64(value); return *this;}
		inline Writer& operator <<(const char * value) { this->mJsonWriter.String(value); return *this;}
		inline Writer& operator <<(const std::string & value) { this->mJsonWriter.String(value.c_str(), value.size()); return *this;}
	public:
		bool StartArray();
		bool StartObject();
		bool StartArray(const char* key);
		bool StartObject(const char* key);
	 public:
		bool EndArray();
		bool EndObject();

	public:
		bool AddMember(long long value);
		bool AddMember(const std::string & value);
	 public:
		bool AddMember(const char* key, XCode code);
	 public:
		bool AddMember(const char* key, int value);
		bool AddMember(const char* key, bool value);
		bool AddMember(const char* key, short value);
		bool AddMember(const char* key, float value);
		bool AddMember(const char* key, double value);
		bool AddMember(const char* key, long long value);
		bool AddMember(const char* key, const char* str);
		bool AddMember(const char* key, unsigned int value);
		bool AddMember(const char* key, unsigned short value);
		bool AddMember(const char* key, const char* str, size_t size);
	 public:
		bool AddMember(const char* key, const std::string& value);
		bool AddMember(const char* key, const std::vector<int>& value);
		bool AddMember(const char* key, const std::vector<long long>& value);
		bool AddMember(const char* key, const std::vector<std::string>& value);
	 public:
		size_t GetJsonSize();
		const std::string ToJsonString();
		size_t WriterStream(std::string& os);
		size_t WriterStream(std::ostream& os);
		bool GetDocument(rapidjson::Document & jsonDocument);
	 private:
		const bool mIsObject;
		rapidjson::StringBuffer mStringBuf;
		rapidjson::Writer<rapidjson::StringBuffer> mJsonWriter;
	};

	class JsonArray
	{

	};
}
#endif //_JSONWRITER_H_

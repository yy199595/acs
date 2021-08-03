#pragma once
#include<memory>
#include<string>
#include<XCode/XCode.h>
#include<rapidjson/writer.h>
#include<rapidjson/document.h>
#include<rapidjson/stringbuffer.h>
namespace Sentry
{
	class QuertJsonWritre
	{
	public:
		QuertJsonWritre();
	public:
		bool StartWriteObject();
		bool StartWriteObject(const char * key);
		bool EndWriteObject();

		bool StartWriteArray(const char * key);
		bool EndWriteArray();

	public:
		bool Write(const char * key);
		bool Write(const char * key, XCode code);
		bool Write(const char * key, double value);
		bool Write(const char * key, long long value);
		bool Write(const char * key, const std::string & value);
		bool Write(const char * key, const char * value, int size);	
	public:
		bool Write();
		bool Write(long long value);
		bool Write(const char * value, int size);
	public:
		bool Serialization(std::string & json);
		bool Serialization(rapidjson::Document & document);
	private:
		rapidjson::StringBuffer mJsonStringBuf;
		rapidjson::Writer<rapidjson::StringBuffer> jsonWriter;
	};
}
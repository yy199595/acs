#pragma once
#include<XCode/XCode.h>
#include<rapidjson/writer.h>
#include<rapidjson/document.h>
#include<rapidjson/stringbuffer.h>
namespace SoEasy
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
		bool Write(const char * key, long long value);
		bool Write(const char * key, const char * value, int size);
	public:
		bool Write();
		bool Write(long long value);
		bool Write(const char * value, int size);
	public:
		bool Serialization(std::string & json);
	private:
		rapidjson::StringBuffer strBuf;
		rapidjson::Writer<rapidjson::StringBuffer> * jsonWriter;
	};
}
namespace SoEasy
{
	class InvokeResultData
	{
	public:
		InvokeResultData(XCode code);
		InvokeResultData(XCode code, const std::string & error);
		InvokeResultData(XCode code, const std::string & error, const std::string & json);
	public:
		bool GetJsonData(rapidjson::Value & jsonData);
		bool GetJsonData(const char * key, rapidjson::Value & jsonData);
	public:
		long long GetInt64();
	public:
		XCode GetCode() { return this->mErrorCode; }
		const std::string & GetErrorStr() { return this->mErrorStr; }
	private:
		XCode mErrorCode;
		std::string mErrorStr;
		rapidjson::Document mDocument;
	};
}
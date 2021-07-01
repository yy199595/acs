#include "InvokeResultData.h"

namespace SoEasy
{
	bool QuertJsonWritre::StartWriteObject()
	{
		return jsonWriter.StartObject();
	}
	bool QuertJsonWritre::StartWriteObject(const char * key)
	{
		jsonWriter.Key(key);
		return jsonWriter.StartObject();
	}
	bool QuertJsonWritre::EndWriteObject()
	{
		return jsonWriter.EndObject();
	}

	bool QuertJsonWritre::StartWriteArray(const char * key)
	{
		jsonWriter.Key(key);
		return jsonWriter.StartArray();
	}
	bool QuertJsonWritre::EndWriteArray()
	{
		return jsonWriter.EndArray();
	}

	bool QuertJsonWritre::Write(const char * key)
	{
		jsonWriter.Key(key);
		return jsonWriter.Null();
	}

	bool QuertJsonWritre::Write(const char * key, XCode code)
	{
		jsonWriter.Key(key);
		return jsonWriter.Int((int)code);
	}

	bool QuertJsonWritre::Write(const char * key, double value)
	{
		jsonWriter.Key(key);
		return jsonWriter.Double(value);
	}

	bool QuertJsonWritre::Write(const char * key, long long value)
	{
		jsonWriter.Key(key);
		return jsonWriter.Int64(value);
	}

	bool QuertJsonWritre::Write(const char * key, const std::string & value)
	{
		jsonWriter.Key(key);
		const char * str = value.c_str();
		const size_t size = value.size();
		return jsonWriter.String(str, size);
	}

	bool QuertJsonWritre::Write(const char * key, const char * value, int size)
	{
		jsonWriter.Key(key);
		if (value == nullptr || size == 0)
		{
			return jsonWriter.String("");
		}
		return jsonWriter.String(value, size);
	}

	bool QuertJsonWritre::Write()
	{
		return jsonWriter.Null();
	}

	bool QuertJsonWritre::Write(long long value)
	{
		return jsonWriter.Int64(value);
	}

	bool QuertJsonWritre::Write(const char * value, int size)
	{
		return jsonWriter.String(value, size);
	}

	bool QuertJsonWritre::Serialization(std::string & json)
	{
		if (this->jsonWriter.EndObject())
		{
			json.assign(this->mJsonStringBuf.GetString(), this->mJsonStringBuf.GetSize());
			return true;
		}
		return false;
	}
	bool QuertJsonWritre::Serialization(rapidjson::Document & document)
	{
		if (this->jsonWriter.EndObject())
		{
			const char * json = this->mJsonStringBuf.GetString();
			const size_t size = this->mJsonStringBuf.GetLength();
			document.Parse(json, size);
			return document.HasParseError() == false;
		}
		return false;
	}
}

namespace SoEasy
{
	InvokeResultData::InvokeResultData(XCode code, const std::string & error, std::shared_ptr<rapidjson::Document> document)
	{
		this->mErrorStr = error;
		this->mErrorCode = code;
		this->mDocument = document;
	}

	InvokeResultData::InvokeResultData(XCode code)
	{
		this->mErrorCode = code;
	}

	InvokeResultData::InvokeResultData(XCode code, const std::string & error)
	{
		this->mErrorCode = code;
		this->mErrorStr = error;
	}

	long long InvokeResultData::GetInt64()
	{
		auto iter = this->mDocument->FindMember("data");
		if (iter != this->mDocument->MemberEnd())
		{
			if (iter->value.IsInt64())
			{
				return iter->value.GetInt64();
			}
		}
		return -1;
	}


	bool InvokeResultData::GetJsonData(rapidjson::Value & jsonData)
	{
		auto iter = this->mDocument->FindMember("data");
		if (iter == this->mDocument->MemberEnd())
		{
			return false;
		}
		jsonData = iter->value;
		return true;
	}

	bool InvokeResultData::GetJsonData(const char * key, rapidjson::Value & jsonData)
	{
		auto iter = this->mDocument->FindMember("data");
		if (iter == this->mDocument->MemberEnd() || iter->value.IsObject() == false)
		{
			return false;
		}
		auto iter1 = iter->value.FindMember(key);
		if (iter1 == iter->value.MemberEnd())
		{
			return false;
		}
		jsonData = iter1->value;
		return true;
	}

	QuertJsonWritre::QuertJsonWritre()
		:jsonWriter(mJsonStringBuf)
	{
		this->jsonWriter.StartObject();
	}
	
}
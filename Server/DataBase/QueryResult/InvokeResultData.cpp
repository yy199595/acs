#include "InvokeResultData.h"


namespace SoEasy
{
	bool QuertJsonWritre::StartWriteObject()
	{
		return jsonWriter->StartObject();
	}
	bool QuertJsonWritre::StartWriteObject(const char * key)
	{
		jsonWriter->Key(key);
		return jsonWriter->StartObject();
	}
	bool QuertJsonWritre::EndWriteObject()
	{
		return jsonWriter->EndObject();
	}

	bool QuertJsonWritre::StartWriteArray(const char * key)
	{
		jsonWriter->Key(key);
		return jsonWriter->StartArray();
	}
	bool QuertJsonWritre::EndWriteArray()
	{
		return jsonWriter->EndArray();
	}

	bool QuertJsonWritre::Write(const char * key)
	{
		jsonWriter->Key(key);
		return jsonWriter->Null();
	}

	bool QuertJsonWritre::Write(const char * key, long long value)
	{
		jsonWriter->Key(key);
		return jsonWriter->Int64(value);
	}

	bool QuertJsonWritre::Write(const char * key, const char * value, int size)
	{
		jsonWriter->Key(key);
		if (value == nullptr || size == 0)
		{
			return jsonWriter->String("");
		}
		return jsonWriter->String(value, size);
	}

	bool QuertJsonWritre::Write()
	{
		return jsonWriter->Null();
	}

	bool QuertJsonWritre::Write(long long value)
	{
		return jsonWriter->Int64(value);
	}

	bool QuertJsonWritre::Write(const char * value, int size)
	{
		return jsonWriter->String(value, size);
	}

	bool QuertJsonWritre::Serialization(std::string & json)
	{
		if (this->jsonWriter->EndObject())
		{
			json.assign(this->strBuf.GetString(), this->strBuf.GetSize());
			return true;
		}
		return false;
	}
}

namespace SoEasy
{
	InvokeResultData::InvokeResultData(XCode code, const std::string & error, const std::string & json)
	{
		this->mErrorStr = error;
		this->mErrorCode = code;
		if (this->mErrorCode == XCode::Successful)
		{
			this->mDocument.Parse(json.c_str(), json.size());
			if (this->mDocument.HasParseError())
			{
				this->mErrorCode = XCode::ParseJsonFailure;
			}
		}
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
		auto iter = this->mDocument.FindMember("data");
		if (iter != this->mDocument.MemberEnd())
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
		auto iter = this->mDocument.FindMember("data");
		if (iter == this->mDocument.MemberEnd())
		{
			return false;
		}
		jsonData = iter->value;
		return true;
	}

	bool InvokeResultData::GetJsonData(const char * key, rapidjson::Value & jsonData)
	{
		auto iter = this->mDocument.FindMember("data");
		if (iter == this->mDocument.MemberEnd() || iter->value.IsObject() == false)
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
	{
		this->strBuf.Clear();
		this->jsonWriter = new rapidjson::Writer<rapidjson::StringBuffer>(strBuf);
		this->jsonWriter->StartObject();
	}
	
}

#include "InvokeResultData.h"

namespace Sentry
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

namespace Sentry
{

	QuertJsonWritre::QuertJsonWritre()
		:jsonWriter(mJsonStringBuf)
	{
		this->jsonWriter.StartObject();
	}
	
}
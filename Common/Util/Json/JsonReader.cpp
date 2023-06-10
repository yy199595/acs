//
// Created by yjz on 2022/3/26.
//

#include"JsonReader.h"

namespace Json
{
	Reader::Reader()
	{

	}

	Reader::Reader(const std::string& json)
		: mJson(std::move(json))
	{
		const char * str = json.c_str();
		const size_t size = json.size();
		this->Parse(str, size);
	}
	Reader::Reader(const char* json, const size_t size)
		: mJson(json, size)
	{
		this->Parse(json, size);
	}

	bool Reader::ParseJson(const std::string& json)
	{
		const char * str = json.c_str();
		const size_t size = json.size();
		this->Parse(str, size);
		return !this->HasParseError();
	}

	bool Reader::ParseJson(const char* json, const size_t size)
	{
		this->Parse(json, size);
		return !this->HasParseError();
	}

	const rapidjson::Value* Reader::GetValue(const char* key) const
	{
		auto iter = this->FindMember(key);
		return iter != this->MemberEnd() ? &iter->value : nullptr;
	}

	bool Reader::GetMember(const char* key, int& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsInt())
		{
			value = jsonValue->GetInt();
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* key, bool& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsBool())
		{
			value = jsonValue->GetBool();
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* key, float& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsFloat())
		{
			value = jsonValue->GetFloat();
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* key, double& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsDouble())
		{
			value = jsonValue->GetDouble();
			return true;
		}
		return false;
	}

	bool Reader::GetMember(const char* key, short& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsInt())
		{
			value = (short )jsonValue->GetInt();
			return true;
		}
		return false;
	}

	bool Reader::GetMember(const char* key, unsigned short& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsUint())
		{
			value = (unsigned short)jsonValue->GetUint();
			return true;
		}
		return false;
	}

	bool Reader::HasMember(const char* key)
	{
		return this->GetValue(key) != nullptr;
	}

	bool Reader::GetMember(const char* key, long long& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsInt64())
		{
			value = jsonValue->GetInt64();
			return true;
		}
		return false;
	}

	bool Reader::GetMember(const char* key, std::string& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsString())
		{
			const char * str = jsonValue->GetString();
			const size_t  size = jsonValue->GetStringLength();
			value.append(str, size);
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* key, unsigned int& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsUint())
		{
			value = jsonValue->GetUint();
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* key, std::vector<int>& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsArray())
		{
			auto jsonArray = jsonValue->GetArray();
			for(int index = 0; index < jsonArray.Size(); index++)
			{
				const rapidjson::Value & data = jsonArray[index];
				if(!data.IsInt())
				{
					return false;
				}
				value.emplace_back(data.GetInt());
			}
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* key, std::vector<long long>& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsArray())
		{
			auto jsonArray = jsonValue->GetArray();
			for(int index = 0; index < jsonArray.Size(); index++)
			{
				const rapidjson::Value & data = jsonArray[index];
				if(!data.IsInt64())
				{
					return false;
				}
				value.emplace_back(data.GetInt64());
			}
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* key, std::vector<std::string>& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(key);
		if(jsonValue != nullptr && jsonValue->IsArray())
		{
			auto jsonArray = jsonValue->GetArray();
			value.reserve(jsonArray.Size());
			for(size_t index = 0; index < jsonArray.Size(); index++)
			{
				const rapidjson::Value & data = jsonArray[index];
				if(!data.IsString())
				{
					return false;
				}
				const char * str = data.GetString();
				const size_t size = data.GetStringLength();
				value.emplace_back(str, size);
			}
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char * k1, std::vector<const rapidjson::Value *> & value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1);
		if(jsonValue->IsArray())
		{
			auto jsonArray = jsonValue->GetArray();
			for(size_t index = 0; index < jsonArray.Size(); index++)
			{
				value.emplace_back(&jsonArray[index]);
			}
			return true;
		}

		return false;
	}

	bool Reader::GetMember(const char* k1, std::unordered_map<std::string, const rapidjson::Value*>& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1);
		if(jsonValue->IsObject())
		{
			auto iter = jsonValue->MemberBegin();
			for(; iter != jsonValue->MemberEnd(); iter ++)
			{
				const rapidjson::Value & val = iter->value;
				value.emplace(iter->name.GetString(), &val);
			}
			return true;
		}
		return false;
	}
}

namespace Json
{
	const rapidjson::Value* Reader::GetValue(const char* k1, const char* k2) const
	{
		auto iter = this->FindMember(k1);
		if(iter == this->MemberEnd() || !iter->value.IsObject())
		{
			return nullptr;
		}
		auto iter1 = iter->value.FindMember(k2);
		return iter1 != iter->value.MemberEnd() ? &iter1->value : nullptr;
	}
	bool Reader::GetMember(const char* k1, const char* k2, int& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsInt())
		{
			value = jsonValue->GetInt();
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, bool& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsBool())
		{
			value = jsonValue->GetBool();
			return true;
		}
		return false;
	}

	bool Reader::GetMember(const char* k1, const char* k2, float& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsFloat())
		{
			value = jsonValue->GetFloat();
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, short& value) const
	{
		const rapidjson::Value* jsonValue = this->GetValue(k1, k2);
		if (jsonValue != nullptr && jsonValue->IsInt())
		{
			value = (short)jsonValue->GetInt();
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, double& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsDouble())
		{
			value = jsonValue->GetDouble();
			return true;
		}
        return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, long long int& value) const
	{
		const rapidjson::Value* jsonValue = this->GetValue(k1, k2);
		if (jsonValue != nullptr && jsonValue->IsInt64())
		{
			value = jsonValue->GetInt64();
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, unsigned int& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsUint())
		{
			value = jsonValue->GetUint();
			return true;
		}
        return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, std::string& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsString())
		{
			const char * str = jsonValue->GetString();
			const size_t size = jsonValue->GetStringLength();
			value.append(str, size);
			return true;
		}
        return false;
    }
	bool Reader::GetMember(const char* k1, const char* k2, std::vector<int>& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsArray())
		{
			auto jsonArray = jsonValue->GetArray();
			for(int index = 0; index < jsonArray.Size(); index++)
			{
				const rapidjson::Value & data = jsonArray[index];
				if(!data.IsInt())
				{
					return false;
				}
				value.emplace_back(data.GetInt());
			}
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, std::vector<long long int>& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsArray())
		{
			auto jsonArray = jsonValue->GetArray();
			for(int index = 0; index < jsonValue->Size(); index++)
			{
				const rapidjson::Value & data = jsonArray[index];
				if(!data.IsInt64())
				{
					return false;
				}
				value.emplace_back(data.GetInt64());
			}
			return true;
		}
        return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, std::vector<std::string>& value) const
	{
		const rapidjson::Value * jsonValue = this->GetValue(k1, k2);
		if(jsonValue != nullptr && jsonValue->IsArray())
		{
			auto jsonArray = jsonValue->GetArray();
			for(int index = 0; index < jsonArray.Size(); index++)
			{
				const rapidjson::Value & data = jsonArray[index];
				if(!data.IsString())
				{
					return false;
				}
				const char * str = data.GetString();
				const size_t size = data.GetStringLength();
				value.emplace_back(str, size);
			}
			return true;
		}
        return false;
	}
}

namespace Json
{
	bool Reader::GetMember(std::vector<int>& value) const
	{
		if (this->IsArray())
		{
			auto jsonArray = this->GetArray();
			for (int index = 0; index < jsonArray.Size(); index++)
			{
				if (!jsonArray[index].IsInt())
				{
					return false;
				}
				value.emplace_back(jsonArray[index].GetInt());
			}
			return true;
		}
		return false;
	}

	bool Reader::GetMember(std::vector<long long>& value) const
	{
		if (this->IsArray())
		{
			auto jsonArray = this->GetArray();
			for (int index = 0; index < jsonArray.Size(); index++)
			{
				if (!jsonArray[index].IsInt64())
				{
					return false;
				}
				value.emplace_back(jsonArray[index].GetInt64());
			}
			return true;
		}
		return false;
	}

	bool Reader::GetMember(std::vector<std::string>& value) const
	{
		if (this->IsArray())
		{
			auto jsonArray = this->GetArray();
			for (int index = 0; index < jsonArray.Size(); index++)
			{
				if (!jsonArray[index].IsString())
				{
					return false;
				}
				const char* str = jsonArray[index].GetString();
				const size_t size = jsonArray[index].GetStringLength();
				value.emplace_back(str, size);
			}
			return true;
		}
		return false;
	}
	bool Reader::GetMember(const char* k1, const char* k2, unsigned short& value) const
	{
		const rapidjson::Value* jsonValue = this->GetValue(k1, k2);
		if (jsonValue != nullptr && jsonValue->IsUint())
		{
			value = (unsigned short)jsonValue->GetUint();
			return true;
		}
		return false;
	}
}
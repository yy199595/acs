//
// Created by yjz on 2022/3/26.
//

#ifndef _JSONREADER_H_
#define _JSONREADER_H_
#include<string>
#include<vector>
#include"XCode/XCode.h"
#include<unordered_map>
#include"rapidjson/document.h"
namespace Json
{
	class Reader : public rapidjson::Document
	{
	 public:
		Reader();
		Reader(const std::string& json);
		Reader(const char* json, const size_t size);
	 public:
		const std::string& GetJson() const
		{
			return this->mJson;
		}
		bool ParseJson(const std::string& json);
		bool ParseJson(const char* json, const size_t size);

	 public:
		bool HasMember(const char * key);
		bool GetMember(const char* key, XCode& code) const;
		bool GetMember(const char* key, int& value) const;
		bool GetMember(const char* key, bool& value) const;
		bool GetMember(const char* key, short& value) const;
		bool GetMember(const char* key, float& value) const;
		bool GetMember(const char* key, double& value) const;
		bool GetMember(const char* key, long long& value) const;
		bool GetMember(const char* key, unsigned int& value) const;
		bool GetMember(const char* key, std::string& value) const;
		bool GetMember(const char* key, unsigned short& value) const;
		bool GetMember(const char* key, std::vector<int>& value) const;
		bool GetMember(const char* key, std::vector<long long>& value) const;
		bool GetMember(const char* key, std::vector<std::string>& value) const;
	 public:
		bool GetMember(const char* k1, const char* k2, int& value) const;
		bool GetMember(const char* k1, const char* k2, bool& value) const;
		bool GetMember(const char* k1, const char* k2, short& value) const;
		bool GetMember(const char* k1, const char* k2, float& value) const;
		bool GetMember(const char* k1, const char* k2, double& value) const;
		bool GetMember(const char* k1, const char* k2, long long& value) const;
		bool GetMember(const char* k1, const char* k2, unsigned int& value) const;
		bool GetMember(const char* k1, const char* k2, unsigned short& value) const;
		bool GetMember(const char* k1, const char* k2, std::string& value) const;
		bool GetMember(const char* k1, const char* k2, std::vector<int>& value) const;
		bool GetMember(const char* k1, const char* k2, std::vector<long long>& value) const;
		bool GetMember(const char* k1, const char* k2, std::vector<std::string>& value) const;
	 public:
		bool GetMember(const char* k1, std::vector<const rapidjson::Value*>& value) const;
		bool GetMember(const char* k1, std::unordered_map<std::string, const rapidjson::Value*>& value) const;

	 public:
		bool GetMember(std::vector<int>& value) const;
		bool GetMember(std::vector<long long>& value) const;
		bool GetMember(std::vector<std::string>& value) const;

	 public:
		const rapidjson::Value* GetJsonValue(const char* key) const;
		const rapidjson::Value* GetJsonValue(const char* k1, const char* k2) const;
	 private:
		std::string mJson;
		//rapidjson::Document mJsonDocument;
	};
}

#endif //_JSONREADER_H_

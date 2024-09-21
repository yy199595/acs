//
// Created by leyi on 2024/4/1.
//
#ifdef __ENABLE_OPEN_SSL__

#include "OssComponent.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/TimeHelper.h"
#include "Util/Crypt/Base64Helper.h"
#include "Http/Client/Http.h"
#include "Util/File/FileHelper.h"

#include <openssl/hmac.h>
#include <openssl/sha.h>


namespace acs
{
	std::string computeSignature(const std::string& key, const std::string& data)
	{
		unsigned char hash[SHA_DIGEST_LENGTH];
		HMAC_CTX* ctx = HMAC_CTX_new();
		HMAC_Init_ex(ctx, key.c_str(), key.length(), EVP_sha1(), NULL);
		HMAC_Update(ctx, (unsigned char*)data.c_str(), data.length());
		unsigned int len = SHA_DIGEST_LENGTH;
		HMAC_Final(ctx, hash, &len);
		HMAC_CTX_free(ctx);
		return std::string(reinterpret_cast<char*>(hash), len);
	}

	OssComponent::OssComponent() = default;

	bool OssComponent::Awake()
	{
		auto& config = this->mApp->Config();
		std::unique_ptr<json::r::Value> ossObject;
		if (!config.Get("oss", ossObject))
		{
			return false;
		}
		ossObject->Get("key", this->mConfig.key);
		ossObject->Get("host", this->mConfig.host);
		ossObject->Get("bucket", this->mConfig.bucket);
		ossObject->Get("region", this->mConfig.region);
		ossObject->Get("secret", this->mConfig.secret);

		return true;
	}

	std::string OssComponent::Sign(oss::Policy& policy)
	{
		json::w::Document document1;
		document1.Add("expiration", help::Time::GetDateISO(policy.expiration));
		auto conditionArray = document1.AddArray("conditions");
		auto jsonArray = conditionArray->AddArray();
		{
			jsonArray->Push("content-length-range");
			jsonArray->Push(0);
			jsonArray->Push(policy.max_length);
		}
		auto jsonArray2 = conditionArray->AddArray();
		{
			jsonArray2->Push("eq");
			jsonArray2->Push("$success_action_status");
			jsonArray2->Push("200");
		}

		auto jsonArray3 = conditionArray->AddArray();
		{
			jsonArray3->Push("in");
			jsonArray3->Push("$content-type");
			auto jsonArray4 = jsonArray3->AddArray();
			{
				for (const std::string& type: policy.limit_type)
				{
					jsonArray4->Push(type);
				}
			}
		}

		std::string str1 = document1.JsonString();
		std::string str2 = help::Base64::Encode(str1);
		std::string str3 = computeSignature(this->mConfig.secret, str2);
		std::string signature = help::Base64::Encode(str3);

		json::w::Document document2;
		document2.Add("ossAccessKeyId", this->mConfig.key);
		document2.Add("host", this->mConfig.host);
		document2.Add("policy", str2);
		document2.Add("signature", signature);

		size_t pos = policy.file_type.find('/');
		if (pos == std::string::npos)
		{
			return "";
		}

		std::string type = policy.file_type.substr(pos + 1);
		const std::string fileName = fmt::format("{}.{}", policy.file_name, type);
		std::string fullName = policy.upload_dir + fileName;

		document2.Add("key", fullName);
		document2.Add("file", fileName);
		document2.Add("url", fmt::format("{}/{}", this->mConfig.host, fullName));
		return document2.JsonString();
	}
}

#endif
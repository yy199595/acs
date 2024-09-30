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
#include "Util/Tools/String.h"
#include "Http/Common/Content.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Http/Component/HttpComponent.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>

constexpr char* OSS_HOST = "https://yy-client.oss-rg-china-mainland.aliyuncs.com";
//constexpr char* OSS_HOST = "http://127.0.0.1:8088/upload/file";

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

	OssComponent::OssComponent()
	{
		this->mHttp = nullptr;
	}

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

	bool OssComponent::LateAwake()
	{
		this->mHttp = this->GetComponent<HttpComponent>();
		return true;
	}

	void OssComponent::Complete()
	{
		this->Upload("C:/Users/64658/Desktop/yy/ace/bin/www/dist/skill_card.png");
	}

	void OssComponent::Upload(const std::string& path)
	{
		std::string fileType;
		oss::Policy ossPolicy;
		help::fs::GetFileType(path, fileType);
		help::Str::GetFileName(path, ossPolicy.file_name);
		{
			ossPolicy.upload_dir = "test/";
			ossPolicy.max_length = 1024 * 1024 * 10;
			ossPolicy.file_type = http::GetContentType(fileType);
			ossPolicy.limit_type.emplace_back(ossPolicy.file_type);
			ossPolicy.expiration = help::Time::NowSec() + (5 * 60);
		}
		oss::FromData fromData;
		this->Sign(ossPolicy, fromData);
		std::unique_ptr<http::Request> httpRequest = std::make_unique<http::Request>("PUT");
		std::unique_ptr<http::MultipartFromContent> fromContent = std::make_unique<http::MultipartFromContent>();
		{
			fromContent->Add("policy", fromData.policy);
			fromContent->Add("OSSAccessKeyId", fromData.OSSAccessKeyId);
			fromContent->Add("success_action_status", "200");
			fromContent->Add("signature", fromData.signature);
			fromContent->Add("key", fromData.key);
		}
		fromContent->Add(path);
		httpRequest->SetUrl(OSS_HOST);
		httpRequest->SetBody(std::move(fromContent));

		http::Response * response = this->mHttp->Do(std::move(httpRequest));
		if (response == nullptr || response->Code() != HttpStatus::OK)
		{

		}
	}

	bool OssComponent::Sign(const oss::Policy& policy, std::string& sign)
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
			return false;
		}

		std::string type = policy.file_type.substr(pos + 1);
		const std::string fileName = fmt::format("{}.{}", policy.file_name, type);
		std::string fullName = policy.upload_dir + fileName;

		document2.Add("key", fullName);
		document2.Add("file", fileName);
		document2.Add("url", fmt::format("{}/{}", this->mConfig.host, fullName));
		return document2.Encode(&sign);
	}

	bool OssComponent::Sign(const oss::Policy& policy, oss::FromData& fromData)
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
				for (const std::string& type : policy.limit_type)
				{
					jsonArray4->Push(type);
				}
			}
		}

		std::string str1 = document1.JsonString();
		std::string str2 = help::Base64::Encode(str1);
		std::string str3 = computeSignature(this->mConfig.secret, str2);

		std::string fullName = policy.upload_dir + policy.file_name;

		fromData.policy = str2;
		fromData.key = fullName;
		fromData.OSSAccessKeyId = this->mConfig.key;
		fromData.signature = help::Base64::Encode(str3);
		fromData.url = fmt::format("{}/{}", this->mConfig.host, fullName);
		return true;
	}
}

#endif
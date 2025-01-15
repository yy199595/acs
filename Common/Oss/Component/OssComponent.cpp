//
// Created by leyi on 2024/4/1.
//
#ifdef __ENABLE_OPEN_SSL__
#include <ctime>
#include <iomanip>
#include "Oss/Lua/LuaOss.h"
#include "Lua/Engine/ModuleClass.h"
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
		return {reinterpret_cast<char*>(hash), len};
	}

	std::string hmac_sha1(const std::string& key, const std::string& data) {
		unsigned char digest[EVP_MAX_MD_SIZE];
		unsigned int digest_len;
		HMAC(EVP_sha1(), key.c_str(), key.length(),
			reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), digest, &digest_len);

		std::string result = std::string(reinterpret_cast<char*>(digest), digest_len);
		return result;
	}

	std::string create_signature(const std::string& method, const std::string& content_md5, const std::string& content_type,
		const std::string& date, const std::string& canonicalized_resource, const std::string& access_key_secret) {
		std::string string_to_sign = method + "\n" + content_md5 + "\n" + content_type + "\n" + date + "\n" + canonicalized_resource;
		std::string signature = hmac_sha1(access_key_secret, string_to_sign);
		return help::Base64::Encode(signature);
	}

	std::string get_utc_time() {
		std::time_t t = std::time(nullptr);
		std::tm* gmt = std::gmtime(&t);
		std::stringstream ss;
		ss << std::put_time(gmt, "%a, %d %b %Y %H:%M:%S GMT");
		return ss.str();
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
		LOG_CHECK_RET_FALSE(!this->mConfig.key.empty());
		LOG_CHECK_RET_FALSE(!this->mConfig.host.empty());
		LOG_CHECK_RET_FALSE(!this->mConfig.bucket.empty());
		LOG_CHECK_RET_FALSE(!this->mConfig.region.empty());
		LOG_CHECK_RET_FALSE(!this->mConfig.secret.empty());
		return true;
	}

	bool OssComponent::LateAwake()
	{
		this->mHttp = this->GetComponent<HttpComponent>();
		return true;
	}

	void OssComponent::Complete()
	{
		//this->Upload("C:/Users/64658/Desktop/yy/ace/bin/config/run/all.json", "10000");
		//this->FromUpload("C:/Users/64658/Desktop/yy/ace/bin/config/run/all.json", "10000");
	}

	std::unique_ptr<http::Request> OssComponent::New(const std::string& path, const std::string& dir)
	{
		std::string fileType, fileName;
		if (!help::fs::GetFileType(path, fileType))
		{
			return nullptr;
		}
		if (!help::Str::GetFileName(path, fileName))
		{
			return nullptr;
		}
		std::string contentType(http::GetContentType(fileType));

		const std::string date = get_utc_time();
		std::string objectKey = fmt::format("{}/{}", dir, fileName);
		std::string url = fmt::format("http://{}.{}.aliyuncs.com/{}",
				this->mConfig.bucket, this->mConfig.region, objectKey);;
		std::string canonicalized_resource = "/" + this->mConfig.bucket + "/" + objectKey;
		std::unique_ptr<http::Request> httpRequest = std::make_unique<http::Request>("PUT");
		{
			httpRequest->SetUrl(url);
			httpRequest->Header().Add("Date", date);
			httpRequest->Header().Add("User-Agent", "acs");
			std::string sign = create_signature("PUT", "", fileType,
					date, canonicalized_resource, this->mConfig.secret);

			std::string auth = fmt::format("OSS {}:{}", this->mConfig.key, sign);
			httpRequest->Header().Add(http::Header::Auth, auth);
		}
		std::unique_ptr<http::FileContent> fileContent = std::make_unique<http::FileContent>();
		{
			if (!fileContent->OpenFile(path, fileType))
			{
				return nullptr;
			}
		}
		httpRequest->SetBody(std::move(fileContent));
		return httpRequest;
	}

	std::unique_ptr<oss::Response> OssComponent::Upload(const std::string& path, const std::string & dir)
	{
		std::unique_ptr<oss::Response> ossResponse = std::make_unique<oss::Response>();
		do
		{
			if (!help::fs::FileIsExist(path))
			{
				ossResponse->data = "file not exist";
				ossResponse->code = HttpStatus::BAD_REQUEST;
				break;
			}
			std::unique_ptr<http::Request> httpRequest = this->New(path, dir);
			if(httpRequest == nullptr)
			{
				ossResponse->data = "create request fail";
				ossResponse->code = HttpStatus::BAD_REQUEST;
				break;
			}
			std::string url = httpRequest->GetUrl().ToStr();
			std::unique_ptr<http::TextContent> textResponse = std::make_unique<http::TextContent>();
			std::unique_ptr<http::Response> response = this->mHttp->Do(std::move(httpRequest), std::move(textResponse));
			if (response == nullptr)
			{
				ossResponse->data = "response is null";
				ossResponse->code = HttpStatus::INTERNAL_SERVER_ERROR;
				break;
			}
			ossResponse->code = response->Code();
			if (response->Code() == HttpStatus::OK)
			{
				ossResponse->url = url;
				break;
			}
			ossResponse->data = response->To<http::TextContent>()->Content();
		} 
		while (false);
		return ossResponse;
	}

	void OssComponent::Sign(const oss::Policy& policy, json::w::Value & document2)
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

		document2.Add("ossAccessKeyId", this->mConfig.key);
		document2.Add("host", this->mConfig.host);
		document2.Add("policy", str2);
		document2.Add("signature", signature);

		size_t pos = policy.file_type.find('/');
		if (pos == std::string::npos)
		{
			return;
		}

		std::string type = policy.file_type.substr(pos + 1);
		const std::string fileName = fmt::format("{}.{}", policy.file_name, type);
		std::string fullName = policy.upload_dir + fileName;

		document2.Add("key", fullName);
		document2.Add("file", fileName);
		document2.Add("url", fmt::format("{}/{}", this->mConfig.host, fullName));
	}

	void OssComponent::Sign(const oss::Policy& policy, oss::FromData& fromData)
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

		std::string fullName = policy.upload_dir + "/" + policy.file_name;

		fromData.policy = str2;
		fromData.key = fullName;
		fromData.OSSAccessKeyId = this->mConfig.key;
		fromData.signature = help::Base64::Encode(str3);
		fromData.url = fmt::format("{}/{}", this->mConfig.host, fullName);
	}
}

#endif
//
// Created by leyi on 2024/4/1.
//

#ifndef APP_ALIOSSCOMPONENT_H
#define APP_ALIOSSCOMPONENT_H

#include"Http/Client/Http.h"
#include "Http/Common/HttpRequest.h"
#include "Entity/Component/Component.h"

#include "AliCloud/Config/Config.h"


namespace oss
{
	struct Object
	{
		int size = 0;
		std::string key;
		long long lastWriteTime;
	};
	struct List
	{
		std::string name;
		std::string prefix;
		std::vector<Object> contents;
	};
}

namespace http
{
	class Response;
}

namespace acs
{
	class AliOssComponent final : public Component
	{
	public:
		AliOssComponent();
		~AliOssComponent() final = default;
	private:
		bool Awake() final;
		bool LateAwake() final;
	public:
		bool Delete(const std::string & objectKey);
		bool Delete(const oss::Config& config, const std::string & objectKey);
	public:
		std::unique_ptr<oss::List> List(const std::string & dir, int count = 100);
		std::unique_ptr<oss::List> List(const oss::Config& config, const std::string & dir, int count = 100);
	public:
		bool Download(const std::string & objectKey, const std::string & path);
		bool Download(const oss::Config& config, const std::string & objectKey, const std::string & path);
	public:
		std::unique_ptr<http::Request> New(const std::string& path, const std::string & dir);
		std::unique_ptr<http::Request> New(const char * method, const oss::Config & config, const std::string& objectKey, bool contentType = false);
	public:
		std::unique_ptr<oss::Response> Upload(const std::string& path, const std::string & dir);
		std::unique_ptr<oss::Response> Upload(const oss::Config & config, const std::string& path, const std::string & dir);
	private:
		std::unique_ptr<http::Response> Run(const std::string & url, std::unique_ptr<http::Request>& request);
	public:
		void Sign(const oss::Policy& policy, oss::FromData & fromData);
		void Sign(const oss::Policy & policy, json::w::Value & document);
	private:
		oss::Config mConfig;
		class HttpComponent* mHttp;
	};
}


#endif

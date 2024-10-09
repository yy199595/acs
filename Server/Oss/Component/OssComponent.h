//
// Created by leyi on 2024/4/1.
//

#ifndef APP_OSSCOMPONENT_H
#define APP_OSSCOMPONENT_H

#ifdef __ENABLE_OPEN_SSL__
#include"Http/Client/Http.h"
#include "Entity/Component/Component.h"

namespace oss
{
	struct Config
	{
		std::string key;
		std::string bucket;
		std::string region;
		std::string secret;
		std::string host;
	};

	struct Policy
	{
		long long expiration;
		long long max_length;
		std::string file_type;
		std::string file_name;
		std::string upload_dir;
		std::vector<std::string> limit_type;
	};

	struct FromData
	{
		std::string policy;
		std::string OSSAccessKeyId;
		std::string signature;
		std::string key;

		std::string url;
	};

	struct Response
	{
		std::string url;
		std::string data;
		HttpStatus code = HttpStatus::OK;
	};
}

namespace acs
{
	class OssComponent : public Component, public IComplete
	{
	public:
		OssComponent();
		~OssComponent() final = default;
	private:
		bool Awake() final;
		bool LateAwake() final;
		void Complete() final;
	public:
		std::unique_ptr<oss::Response> Upload(const std::string& path, const std::string & dir);
		std::unique_ptr<oss::Response> FromUpload(const std::string& path, const std::string& dir);
	public:
		bool Sign(const oss::Policy & policy, std::string & sign);
		bool Sign(const oss::Policy& policy, oss::FromData & fromData);
	private:
		oss::Config mConfig;
		class HttpComponent* mHttp;
	};
}


#endif //__ENABLE_OPEN_SSL__

#endif // APP_OSSCOMPONENT_H

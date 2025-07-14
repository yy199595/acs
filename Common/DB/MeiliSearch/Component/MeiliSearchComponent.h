//
// Created by 64658 on 2025/5/30.
//

#ifndef APP_MEILISEARCHCOMPONENT_H
#define APP_MEILISEARCHCOMPONENT_H

#include "Http/Common/Content.h"
#include "Entity/Component/Component.h"
#include "MeiliSearch/Config/MsConfig.h"


namespace ms
{
	namespace search
	{
		struct Request
		{
		public:
			int offset = 0;
			int limit = 0;
			std::string q;
			std::string filter;
			std::vector<std::string> sort;
			std::vector<std::string> fields;
		};

		struct Response
		{
		public:
			std::string query;
			unsigned int processingTimeMs = 0;
			unsigned int estimatedTotalHits = 0;
			std::vector<json::r::Value> hits;
		public:
			std::unique_ptr<http::JsonContent> httpContent;
		};

		template<typename T>
		struct Result
		{
		public:
			std::string query;
			std::vector<T> hits;
			unsigned int processingTimeMs = 0;
			unsigned int estimatedTotalHits = 0;
		};
	}

	namespace setting
	{
		struct Request
		{
		public:
			std::string distinct;
			std::vector<std::string> sort;
			std::vector<std::string> search; //搜索字段
			std::vector<std::string> filter; //过滤字段
			std::vector<std::string> ranking; //排序规则
		};
	}

	namespace index
	{
		struct IndexInfo
		{
		public:
			std::string uid;
			std::string createdAt;
			std::string updatedAt;
			std::string primaryKey;
		};

		struct Response
		{
		public:
			unsigned int total;
			unsigned int limit;
			unsigned int offset;
			std::vector<IndexInfo> results;
		};
	}
}


namespace acs
{
	class MeiliSearchComponent : public Component
	{
	public:
		MeiliSearchComponent();
		~MeiliSearchComponent() final = default;
	private:
		bool LateAwake() final;
	public:
		std::unique_ptr<ms::index::Response> Indexes();
		bool Create(const std::string &uid, const std::string &key);
		bool Set(const std::string &uid, json::w::Document & document);
		bool Setting(const std::string & uid, const ms::setting::Request & setting);
		std::unique_ptr<ms::search::Response> Search(const std::string &uid, const ms::search::Request & search);
	private:
		std::unique_ptr<http::JsonContent> Do(const char * method, const std::string & url);
		std::unique_ptr<http::JsonContent> Do(const char * method, const std::string & url, json::w::Document & request);
	public:
		template<typename T>
		inline std::unique_ptr<ms::search::Result<T>> Search(const std::string &uid, const ms::search::Request & search)
		{
			typedef ms::search::Result<T> Result;
			std::unique_ptr<ms::search::Response> response = this->Search(uid, search);
			if(response == nullptr)
			{
				return nullptr;
			}
			std::unique_ptr<Result> result = std::make_unique<Result>();
			{
				result->query = response->query;
				result->processingTimeMs = response->processingTimeMs;
				result->estimatedTotalHits = response->estimatedTotalHits;
				for(const json::r::Value & jsonValue : response->hits)
				{
					T value;
					if(value.Decode(jsonValue))
					{
						result->hits.emplace_back(value);
					}
				}
			}
			return result;
		}
	private:
		ms::Config mConfig;
		std::string mAuthorization;
		class HttpComponent * mHttp;
	};
}

#endif //APP_MEILISEARCHCOMPONENT_H

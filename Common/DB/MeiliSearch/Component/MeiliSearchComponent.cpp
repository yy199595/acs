//
// Created by 64658 on 2025/5/30.
//

#include "MeiliSearchComponent.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/TimeHelper.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Http/Component/HttpComponent.h"


namespace acs
{
	MeiliSearchComponent::MeiliSearchComponent()
	{
		this->mHttp = nullptr;
		REGISTER_JSON_CLASS_MUST_FIELD(ms::Config, key);
		REGISTER_JSON_CLASS_MUST_FIELD(ms::Config, address);
	}

	bool MeiliSearchComponent::LateAwake()
	{
		ServerConfig & config = this->mApp->GetConfig();
		this->mHttp = this->GetComponent<HttpComponent>();
		LOG_CHECK_RET_FALSE(config.Get("ms", this->mConfig));
		this->mAuthorization = fmt::format("Bearer {}", this->mConfig.key);
		return true;
	}

	bool MeiliSearchComponent::Set(const std::string& uid, json::w::Document& document)
	{
		const std::string url = fmt::format("{}/indexes/{}/documents", this->mConfig.address, uid);
		std::unique_ptr<http::JsonContent> jsonContent = this->Do("POST", url, document);
		if(jsonContent == nullptr)
		{
			return false;
		}
		return true;
	}

	std::unique_ptr<http::JsonContent> MeiliSearchComponent::Do(const char* method, const std::string& url)
	{
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>(method);
		if(!request->SetUrl(url))
		{
			return nullptr;
		}
		request->Header().Add(http::Header::Auth, this->mAuthorization);
		std::unique_ptr<http::Response> response = this->mHttp->Do(request);
		if(response == nullptr || response->GetBody() == nullptr || response->Code() != HttpStatus::OK)
		{
			return nullptr;
		}
		if(response->GetBody()->GetContentType() != http::ContentType::JSON)
		{
			return nullptr;
		}
		std::unique_ptr<http::Content> httpContent = response->MoveBody();
		if(httpContent->GetContentType() != http::ContentType::JSON)
		{
			return nullptr;
		}
		return std::unique_ptr<http::JsonContent>((http::JsonContent*)httpContent.release());
	}

	std::unique_ptr<http::JsonContent> MeiliSearchComponent::Do(
			const char* method,const std::string& url, json::w::Document& document)
	{
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>(method);
		if(!request->SetUrl(url))
		{
			return nullptr;
		}
		request->SetContent(document);
		request->Header().Add(http::Header::Auth, this->mAuthorization);
		std::unique_ptr<http::Response> response = this->mHttp->Do(request);
		if(response == nullptr || response->GetBody() == nullptr || response->Code() != HttpStatus::OK)
		{
			return nullptr;
		}
		if(response->GetBody()->GetContentType() != http::ContentType::JSON)
		{
			return nullptr;
		}
		std::unique_ptr<http::Content> httpContent = response->MoveBody();
		if(httpContent->GetContentType() != http::ContentType::JSON)
		{
			return nullptr;
		}
		return std::unique_ptr<http::JsonContent>((http::JsonContent*)httpContent.release());
	}

	bool MeiliSearchComponent::Setting(const std::string& uid, const ms::setting::Request& setting)
	{
		const std::string url = fmt::format("{}/indexes/{}/settings", this->mConfig.address, uid);
		{
			json::w::Document document;
			document.Add("sortableAttributes", setting.sort, false);
			document.Add("searchableAttributes", setting.search, false);
			document.Add("filterableAttributes", setting.filter, false);
			if(!setting.ranking.empty())
			{
				document.Add("rankingRules", setting.ranking);
			}
			std::unique_ptr<http::JsonContent> jsonContent = this->Do("PUT", url, document);
			if(jsonContent == nullptr)
			{
				return false;
			}
		}
		return true;
	}

	bool MeiliSearchComponent::Create(const std::string& uid, const std::string& key)
	{
		json::w::Document document;
		const std::string url = fmt::format("{}/indexes", this->mConfig.address);
		{
			if(!key.empty())
			{
				document.Add("primaryKey", key);
			}
			document.Add("uid", uid);
		}
		std::unique_ptr<http::JsonContent> jsonContent = this->Do("POST", url, document);
		if(jsonContent == nullptr)
		{
			return false;
		}
		return true;
	}

	std::unique_ptr<ms::search::Response> MeiliSearchComponent::Search(const std::string& uid, const ms::search::Request& search)
	{
		json::w::Document document;
		const std::string url = fmt::format("{}/indexes/{}/search", this->mConfig.address, uid);
		{
			document.Add("q", search.q);
			if(!search.filter.empty())
			{
				document.Add("filter", search.filter);
			}
			document.Add("sort", search.sort);
			if(search.limit > 0)
			{
				document.Add("limit", search.limit);
			}
			document.Add("offset", search.offset);
			document.Add("attributesToRetrieve", search.fields, false);
		}
		std::unique_ptr<http::JsonContent> jsonContent = this->Do("POST", url, document);
		if(jsonContent == nullptr)
		{
			return nullptr;
		}
		std::unique_ptr<ms::search::Response> searchResponse = std::make_unique<ms::search::Response>();
		{
			json::r::Value hitValue;
			json::r::Value jsonValue;
			LOG_CHECK_RET_NULL(jsonContent->Get("hits", hitValue))
			size_t count = hitValue.MemberCount();
			searchResponse->hits.reserve(count);
			for (size_t index = 0; index < count; index++)
			{
				if (!hitValue.Get(index, jsonValue))
				{
					continue;
				}
				searchResponse->hits.emplace_back(jsonValue.GetValue());
			}
			LOG_CHECK_RET_NULL(jsonContent->Get("query", searchResponse->query))
			LOG_CHECK_RET_NULL(jsonContent->Get("processingTimeMs", searchResponse->processingTimeMs));
			LOG_CHECK_RET_NULL(jsonContent->Get("estimatedTotalHits", searchResponse->estimatedTotalHits));
		}
		searchResponse->httpContent = std::move(jsonContent);
		return searchResponse;
	}

	std::unique_ptr<ms::index::Response> MeiliSearchComponent::Indexes()
	{
		std::string url = fmt::format("{}/indexes", this->mConfig.address);
		std::unique_ptr<ms::index::Response> indexes = std::make_unique<ms::index::Response>();
		std::unique_ptr<http::JsonContent> httpContent1 = this->Do("GET", url);
		{
			json::r::Value jsonArray;
			httpContent1->Get("limit", indexes->limit);
			httpContent1->Get("total", indexes->total);
			httpContent1->Get("offset", indexes->offset);
			if(httpContent1->Get("results", jsonArray))
			{
				json::r::Value jsonItem;
				size_t count = jsonArray.MemberCount();
				for(size_t index = 0; index < count; index++)
				{
					if(jsonArray.Get(index, jsonItem))
					{
						ms::index::IndexInfo indexInfo;
						jsonItem.Get("uid", indexInfo.uid);
						jsonItem.Get("createdAt", indexInfo.createdAt);
						jsonItem.Get("updatedAt", indexInfo.updatedAt);
						jsonItem.Get("primaryKey", indexInfo.primaryKey);
						indexes->results.emplace_back(indexInfo);
					}
				}
			}
		}
		return indexes;
	}
}
//
// Created by yy on 2024/6/23.
//

#include "GroupNotifyComponent.h"
#include "HttpComponent.h"
#include "Entity/Actor/App.h"
#include "Lua/Engine/ModuleClass.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Util/Tools/String.h"
namespace acs
{
	std::unique_ptr<http::Request> Make(const std::string & url,
			const json::w::Document & message, const char * type)
	{
		json::w::Document document;
		document.Add("msgtype", type);
		document.Add(type, message);
//		std::unique_ptr<json::w::Value> jsonValue = document.AddObject(type);
//		{
//			jsonValue->Add("content", message);
//		}
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("POST");
		{
			if (!request->SetUrl(url))
			{
				return nullptr;
			}
			request->SetContent(document);
		}
		return request;
	}

	GroupNotifyComponent::GroupNotifyComponent()
	{
		this->mHttp = nullptr;
	}

	bool GroupNotifyComponent::Awake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		LOG_CHECK_RET_FALSE(this->mApp->Config().Get("notify", jsonObject))
		{
			LOG_CHECK_RET_FALSE(jsonObject->Get("wx", this->mWxUrl))
			LOG_CHECK_RET_FALSE(jsonObject->Get("ding", this->mDingUrl))
		}
		LOG_CHECK_RET_FALSE(help::Str::IsHttpAddr(this->mWxUrl))
		LOG_CHECK_RET_FALSE(help::Str::IsHttpAddr(this->mDingUrl))
		return true;
	}

	bool GroupNotifyComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mHttp = this->GetComponent<HttpComponent>())
		return true;
	}

	void GroupNotifyComponent::SendToWeChat(const std::string & message)
	{
		json::w::Document document;
		document.Add("content", message);
		std::unique_ptr<http::Request> request = acs::Make(this->mWxUrl, document, "text");
		{
			this->mHttp->Send(std::move(request), [](http::Response* response)
			{
				CONSOLE_LOG_INFO("{}", response->GetBody()->ToStr());
			});
		};
	}

	void GroupNotifyComponent::SendToWeChat(const notify::Markdown& message)
	{
		if(this->mWxUrl.empty() || this->mHttp == nullptr)
		{
			return;
		}
		json::w::Document document;
		std::string content = fmt::format("# {}\n", message.title);
		std::string url = message.url.empty() ? this->mWxUrl : message.url;

		for(const notify::Data & data : message.data)
		{
			content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", data.key, data.value));
		}

		document.Add("content", content);
		auto request = acs::Make(url, document, "markdown");
		{
			this->mHttp->Send(std::move(request), [](http::Response* response)
			{
				CONSOLE_LOG_FATAL("{}", response->GetBody()->ToStr());
			});
		}
	}

	void GroupNotifyComponent::SendToWeChat(const notify::TemplateCard& message, bool async)
	{
		if (this->mWxUrl.empty() || this->mHttp == nullptr)
		{
			return;
		}
		json::w::Document document;
		document.Add("card_type", "text_notice");
		auto sourceObject = document.AddObject("source");
		{
			sourceObject->Add("icon_url",
					"https://lf16-fe.bytedgame.com/obj/gamefe-sg/toutiao/fe/game_interface_platform_fe/icon.png");
			sourceObject->Add("desc", "服务器通知");
			sourceObject->Add("desc_color", 0);
		}
		auto mainObejct = document.AddObject("main_title");
		{
			mainObejct->Add("title", message.title);
			//mainObejct->Add("desc", "服务器代码报错啦");
		}
		auto jsonArray = document.AddArray("horizontal_content_list");
		for (const notify::Data& item: message.data)
		{
			auto jsonObejct = jsonArray->AddObject();
			{
				jsonObejct->Add("keyname", item.key);
				jsonObejct->Add("value", item.value);
			}
		}

		auto actionObject = document.AddObject("card_action");
		{
			if (message.Jump.appid.empty())
			{
				actionObject->Add("type", 1);
				actionObject->Add("url", "https://huwai.pro");
			}
			else
			{
				actionObject->Add("type", 2);
				actionObject->Add("appid", message.Jump.appid);
				actionObject->Add("pagepath", message.Jump.page);
			}
		}
		std::string url = message.url.empty() ? this->mWxUrl : message.url;
		auto request = acs::Make(url, document, "template_card");
		if (async)
		{
			this->mHttp->Do(std::move(request));
			return;
		}
		this->mHttp->Send(std::move(request), [](http::Response* response)
		{

		});

	}

	void GroupNotifyComponent::SendToDingDing(const std::string & message)
	{
		json::w::Document document;
		document.Add("content", message);
		std::unique_ptr<http::Request> request = acs::Make(this->mDingUrl, document, "text");
		{
			this->mHttp->Send(std::move(request), [](http::Response* response)
			{
				CONSOLE_LOG_INFO("{}", response->GetBody()->ToStr());
			});
		};
	}

	void GroupNotifyComponent::OnLuaRegister(Lua::ModuleClass& luaRegister)
	{

	}
}
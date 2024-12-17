//
// Created by leyi on 2023/12/25.
//

#include "WeChatOutput.h"

#include <utility>
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Util/Tools/TimeHelper.h"

namespace custom
{
	WeChatOutput::WeChatOutput(std::string  url, std::string  pem)
			: mUrl(std::move(url))
#ifdef __ENABLE_OPEN_SSL__
			, mPem(std::move(pem)), mCtx(asio::ssl::context::sslv23)
#endif
	{
	}

	void WeChatOutput::Push(Asio::Context& io, const std::string& name, const custom::LogInfo& log)
	{
		json::w::Document message;
		{
			std::string time = help::Time::GetDateString();
			std::string content = fmt::format("# {}\n", "日志通知");

			switch (log.Level)
			{
			case custom::LogLevel::Debug:
				content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "等级", "debug"));
				break;
			case custom::LogLevel::Info:
				content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "等级", "info"));
				break;
			case custom::LogLevel::Warn:
				content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "等级", "warn"));
				break;
			case custom::LogLevel::Error:
				content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "等级", "error"));
				break;
			case custom::LogLevel::Fatal:
				content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "等级", "fatal"));
				break;
			}
			content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "时间", time));
			content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "文件", log.File));
			content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "内容", log.Content));

			if(!log.Stack.empty())
			{
				content.append(fmt::format("<font color=comment> **{}** </font>  {}\n", "堆栈", log.Stack));
			}
			message.Add("content", content);
		}
		json::w::Document document;
		document.Add("msgtype", "markdown");
		document.Add("markdown", message);
#ifdef __ENABLE_OPEN_SSL__
		this->mClient->SetSocket(new tcp::Socket(io, this->mCtx));
#else
		this->mClient->SetSocket(new tcp::Socket(io));
#endif
		http::Response response;
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("POST");
		{
			request->SetUrl(this->mUrl);
			request->SetContent(document);
			this->mClient->SyncSend(std::move(request), response);
		}
	}

	bool WeChatOutput::Start(Asio::Context& io)
	{
		Asio::Code code;
#ifdef __ENABLE_OPEN_SSL__
		this->mCtx.load_verify_file(this->mPem, code);
#endif
		this->mClient = std::make_shared<http::RequestClient>(nullptr, io);
		return code.value() == 0;
	}
}
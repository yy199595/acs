//
// Created by leyi on 2023/12/25.
//

#include "PushOutput.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Util/Time/TimeHelper.h"

namespace custom
{
	PushOutput::PushOutput(const std::string& url, const std::string & pem)
		: mUrl(url)
#ifdef __ENABLE_OPEN_SSL__
		, mPem(pem), mCtx(asio::ssl::context::sslv23)
#endif
	{

	}

	void PushOutput::Push(Asio::Context &io, const std::string& name, const custom::LogInfo& log)
	{
		if(log.Level < custom::LogLevel::Error)
		{
			return;
		}
		json::w::Document document;
		document.Add("msgtype", "text");
		std::unique_ptr<json::w::Value> jsonObj = document.AddObject("text");
		{
			std::string time = help::Time::GetDateString();
			switch (log.Level)
			{
				case custom::LogLevel::Error:
					jsonObj->Add("content", fmt::format("{} [{}:Error] {} {}", time, name, log.File, log.Content));
					break;
				case custom::LogLevel::Fatal:
					jsonObj->Add("content", fmt::format("{} [{}:Fatal] {} {}", time, name, log.File, log.Content));
					break;
			}
		}
		std::string text;
		document.Encode(&text);
#ifdef __ENABLE_OPEN_SSL__
		this->mClient->SetSocket(new tcp::Socket(io, this->mCtx));
#else
		this->mClient->SetSocket(new tcp::Socket(io));
#endif
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("POST");
		if (!request->SetUrl(this->mUrl))
		{
			return;
		}
		request->SetTimeout(15);
		request->SetContent(http::Header::JSON, text);
		this->mClient->Do(std::move(request), std::move(response), 0);
	}

	bool PushOutput::Start(Asio::Context& io)
	{
		Asio::Code code;
#ifdef __ENABLE_OPEN_SSL__
		this->mCtx.load_verify_file(this->mPem, code);
#endif
		this->mClient = std::make_unique<http::RequestClient>(nullptr);
		return code.value() == 0;
	}
}
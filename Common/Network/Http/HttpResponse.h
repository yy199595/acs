#pragma once
#include<string>
#include<XCode/XCode.h>
#include<Define/CommonTypeDef.h>
namespace Sentry
{
	class HttpResponse
	{
	public:
		HttpResponse() { }
		~HttpResponse() { }
	public:
		int ParseHttpResponse(asio::streambuf & strem);
	public:
		int GetCode() { return this->mCode; }
		XCode GetResponse(std::string & response);
		const std::string & GetVersion() { return this->mVersion; }		
	private:
		int mCode;
		std::string mData;
		std::string mVersion;
	};
}
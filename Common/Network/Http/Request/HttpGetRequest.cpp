#include "HttpGetRequest.h"
#include <Network/NetworkHelper.h>
#include <Network/Http/HttpLocalsession.h>
#include<Core/App.h>

namespace Sentry
{
	HttpGetRequest::HttpGetRequest(HttpClientComponent * component)
		:mHttpComponent(component)
	{
		this->mCorId = 0;
		this->mResponse = nullptr;
	}

	XCode HttpGetRequest::Get(const std::string & url, std::string & response)
	{	
		if (!NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath))
		{
			return XCode::HttpUrlParseError;
		}
		this->mResponse = &response;
		HttpLocalSession httpSession(mHttpComponent, this);
		httpSession.StartConnectHost(this->mHost, this->mPort);
		App::Get().GetCorComponent()->YieldReturn(this->mCorId);
		if (this->mCode != XCode::Successful)
		{
			return this->mCode;
		}
		return this->mHttpCode != 200 ? XCode::HttpResponseError : XCode::Successful;
	}

	bool HttpGetRequest::WriterToBuffer(std::ostream & os)
	{
		os << "GET " << mPath << " HTTP/1.0\r\n";
		os << "Host: " << mHost << "\r\n";
		os << "Accept: */*\r\n";
		os << "Connection: close\r\n\r\n";
		return true;
	}

	bool HttpGetRequest::OnSessionError(const asio::error_code & code)
	{
#ifdef __DEBUG__
		SayNoDebugError(code.message());
#endif // __DEBUG__
		this->mCode = XCode::HttpNetWorkError;
		return false;
	}

	bool HttpGetRequest::OnReceiveBody(asio::streambuf & buf, const asio::error_code & code)
	{
		if (code == asio::error::eof)
		{
			this->mCode = XCode::Successful;
			return false;
		}
		else if (code)
		{
			this->mCode = XCode::HttpNetWorkError;
			return false;
		}
		std::istream is(&buf);
		std::istreambuf_iterator<char> eos;
		this->mResponse->append(std::istreambuf_iterator<char>(is), eos);
		return true;

	}

	bool HttpGetRequest::OnReceiveHeard(asio::streambuf & buf, const asio::error_code & code)
	{
		std::istream is(&buf);
		is >> this->mHttpVersion >> this->mHttpCode >> this->mHttpError;
		this->ParseHeard(buf);
		return this->mHttpCode == 200;
	}

}
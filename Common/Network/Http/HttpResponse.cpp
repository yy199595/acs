#include"HttpResponse.h"
#include<istream>
#include<XCode/XCode.h>
namespace Sentry
{
	int HttpResponse::ParseHttpResponse(asio::streambuf & strem)
	{
		std::istream is(&strem);
		is >> this->mVersion >> this->mCode;
		while (std::getline(is, this->mData))
		{
			if (this->mData.length() == 0 && this->mData == "\r")
			{
				break;
			}
		}
		return this->mCode;
	}
	XCode HttpResponse::GetResponse(std::string & response)
	{
		if (this->mCode != 200)
		{
			return XCode::HttpResponseError;
		}
		response = this->mData;
		return XCode::Successful;
	}
}

//
// Created by yy on 2023/12/10.
//

#include"Url.h"
#include<regex>
#include"sstream"
#include"Log/Common/CommonLogDef.h"
namespace http
{
	Url::Url() : mVersion(http::Version), mReadCount(0) { }

	Url::Url(const char* method)
		: mVersion(http::Version),
		  mMethod(method), mReadCount(0)
	{

	}

	int Url::OnSendMessage(std::ostream& os)
	{
		if(this->mPath.empty())
		{
			this->mPath = "/";
		}
		os << this->mMethod << " " << this->mPath
			<< " " << this->mVersion << http::CRLF;
		return 0;
	}

	void Url::Clear()
	{
		this->mReadCount = 0;
		this->mUrl.clear();
		this->mPort.clear();
		this->mHost.clear();
		this->mPath.clear();
		this->mQuery.Clear();
		this->mProto.clear();
		this->mMethod.clear();
		this->mVersion = http::Version;
	}

	int Url::OnRecvMessage(std::istream& os, size_t size)
	{
		this->mReadCount++;
		if(this->mReadCount >= 3 || size <= 2)
		{
			return tcp::ReadDecodeError;
		}

		if(!std::getline(os, this->mMethod, ' '))
		{
			return tcp::ReadDecodeError;
		}
		if(!std::getline(os, this->mUrl, ' '))
		{
			return tcp::ReadDecodeError;
		}
		if(!std::getline(os, this->mVersion))
		{
			return tcp::ReadDecodeError;
		}
		if(this->mVersion.back() == '\r')
		{
			this->mVersion.pop_back();
		}
		if(this->mMethod.empty() || this->mUrl.empty() || this->mVersion.empty())
		{
			return tcp::ReadDecodeError;
		}
		size_t pos = this->mUrl.find('?');
		if(pos == std::string::npos)
		{
			this->mPath = this->mUrl;
			return tcp::ReadDone;
		}
		this->mPath = this->mUrl.substr(0, pos);
		std::string query = this->mUrl.substr(pos + 1);
		return this->mQuery.Decode(query) ? tcp::ReadDone : tcp::ReadDecodeError;
	}


	bool Url::Decode(const std::string& url)
	{
		std::cmatch what;
		std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
		if (std::regex_match(url.c_str(), what, pattern))
		{
			this->mHost = std::string(what[2].first, what[2].second);
			this->mPath = std::string(what[4].first, what[4].second);
			this->mProto.append(what[1].first, what[1].second);
			this->mPort = std::string(what[3].first, what[3].second);
			if(this->mPath.empty())
			{
				this->mPath = "/";
			}
			if (this->mPort.empty())
			{
				this->mPort = this->IsHttps() ? "443" : "80";
			}
			this->mUrl = url;
			return !this->mHost.empty();
		}
		return false;
	}
}

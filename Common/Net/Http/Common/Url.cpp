//
// Created by yy on 2023/12/10.
//

#include"Url.h"
#include<regex>
#include"sstream"
#include"Util/String/String.h"
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
		os << this->mMethod << " " << this->mUrl
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
		if(this->mReadCount >= 3 || size == 0)
		{
			LOG_ERROR("read:{} size:{}", this->mReadCount, size);
			return tcp::ReadDecodeError;
		}
		this->mReadCount++;
		std::unique_ptr<char[]> buffer(new char[size]);
		size_t count = os.readsome(buffer.get(), size);
		if(count != size)
		{
			LOG_ERROR("read:{} size:{} count:{}", count, size, this->mReadCount);
			return tcp::ReadDecodeError;
		}
		std::string lineData(buffer.get(), size - 2);

		std::vector<std::string> results;
		results.reserve(3);
		if(help::Str::Split(lineData, ' ', results) != 3)
		{
			return tcp::ReadDecodeError;
		}
		this->mUrl = std::move(results[1]);
		this->mMethod = std::move(results[0]);
		this->mVersion = std::move(results[2]);
		if(this->mMethod.empty() || this->mUrl.empty() || this->mVersion.empty())
		{
			LOG_ERROR("line data : {}", lineData);
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
			this->mUrl = std::string(what[4].first, what[4].second);
			this->mProto.append(what[1].first, what[1].second);
			this->mPort = std::string(what[3].first, what[3].second);
			if(this->mUrl.empty())
			{
				this->mUrl = "/";
			}
			if (0 == this->mPort.length())
			{
				this->mPort = this->IsHttps() ? "443" : "80";
			}
			return !this->mHost.empty();
		}
		return false;
	}
}

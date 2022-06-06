//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpAsyncRequest.h"
#include<regex>
#include"Http.h"
#include<iostream>
#include"Util/StringHelper.h"
namespace Sentry
{
	HttpAsyncRequest::HttpAsyncRequest(const std::string& method)
		: mMethod(method)
	{

	}

    bool HttpAsyncRequest::ParseUrl(const std::string &url)
    {
        std::cmatch what;
        std::string protocol;
        std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
        if (std::regex_match(url.c_str(), what, pattern))
		{
            this->mHost = std::string(what[2].first, what[2].second);
            this->mPath = std::string(what[4].first, what[4].second);
            protocol = std::string(what[1].first, what[1].second);
            this->mPort = std::string(what[3].first, what[3].second);

            if (0 == this->mPort.length()) {
                this->mPort = "http" == protocol ? "80" : "443";
            }
            return true;
        }
        return false;
    }

	bool HttpAsyncRequest::AddHead(const std::string& key, int value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, std::to_string(value));
		return true;
	}

	bool HttpAsyncRequest::AddHead(const std::string& key, const std::string& value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, value);
		return true;
	}

	int HttpAsyncRequest::Serailize(std::ostream& os)
	{
		os << this->mMethod << " " << this->mPath << " " << HttpVersion << "\r\n";
		os << "Host:" << this->mHost << "\r\n";
		for(auto iter = this->mHeadMap.begin(); iter != this->mHeadMap.end(); iter++)
		{
			const std::string & key = iter->first;
			const std::string & value = iter->second;
			os << key << ':' << value << "\r\n";
		}
		os << "Connection: close\r\n\r\n";
		this->WriteBody(os);
		return 0;
	}
}


namespace Sentry
{
	HttpGetRequest::HttpGetRequest()
		: HttpAsyncRequest("GET")
	{

	}

	void HttpGetRequest::WriteBody(std::ostream& os)
	{

	}

	std::shared_ptr<HttpGetRequest> HttpGetRequest::Create(const std::string& url)
	{
		std::shared_ptr<HttpGetRequest> request(new HttpGetRequest());
		return request->ParseUrl(url) ? request : nullptr;
	}
}

namespace Sentry
{
	HttpPostRequest::HttpPostRequest()
		: HttpAsyncRequest("POST")
	{

	}

	void HttpPostRequest::AddBody(const std::string& content)
	{
		this->mBody = content;
		this->AddHead("content-length", (int)content.size());
	}

	void HttpPostRequest::WriteBody(std::ostream& os)
	{
		os.write(this->mBody.c_str(), this->mBody.size());
	}

	std::shared_ptr<HttpPostRequest> HttpPostRequest::Create(const std::string& url)
	{
		std::shared_ptr<HttpPostRequest> request(new HttpPostRequest());
		return request->ParseUrl(url) ? request : nullptr;
	}

	void HttpPostRequest::AddBody(const char* data, size_t size)
	{
		this->mBody.append(data, size);
		this->AddHead("content-length", size);
	}
}

namespace Sentry
{
	HttpAsyncResponse::HttpAsyncResponse(std::fstream* fs)
	{
		this->mHttpCode = 0;
		this->mFstream = fs;
		this->mState = HttpDecodeState::FirstLine;
	}

	HttpAsyncResponse::~HttpAsyncResponse()
	{
		if(this->mFstream != nullptr)
		{
			this->mFstream->close();
			delete this->mFstream;
		}
	}

    HttpStatus HttpAsyncResponse::OnReceiveData(asio::streambuf &streamBuffer)
	{
		std::iostream io(&streamBuffer);
		if (this->mState == HttpDecodeState::FirstLine)
		{
			std::string version;
			this->mState = HttpDecodeState::HeadLine;
			io >> version >> this->mHttpCode >> this->mHttpError;
			this->mHttpData.set_version(version);
			io.ignore(2); //放弃\r\n
		}
		if (this->mState == HttpDecodeState::HeadLine)
		{
			std::string lineData;
			while (std::getline(io, lineData))
			{
				if (lineData == "\r")
				{
					this->mState = HttpDecodeState::Content;
					break;
				}
				size_t pos = lineData.find(':');
				if (pos != std::string::npos)
				{
					size_t length = lineData.size() - pos - 2;
					std::string key = lineData.substr(0, pos);
					Helper::String::Tolower(key);
					std::string val = lineData.substr(pos + 1, length);
					this->mHttpData.mutable_head()->insert({key, val});
				}
			}
		}
		if (this->mState == HttpDecodeState::Content)
		{
			char buffer[256] = { 0 };
			while (io.readsome(buffer, 256) > 0)
			{
				if (this->mFstream != nullptr)
				{
					this->mFstream->write(buffer, io.gcount());
					continue;
				}
				this->mHttpData.mutable_data()->append(buffer, io.gcount());
			}
		}
		return HttpStatus::CONTINUE;
	}

	bool HttpAsyncResponse::GetHead(const std::string& key, std::string& value)
	{
		auto iter = this->mHttpData.head().find(key);
		if(iter != this->mHttpData.head().end())
		{
			value = iter->second;
			return true;
		}
		return false;
	}
}

namespace Sentry
{
    HttpHandlerRequest::HttpHandlerRequest(const std::string & address)
    {
		this->mHttpData.set_address(address);
        this->mState = HttpDecodeState::FirstLine;
    }

    bool HttpHandlerRequest::GetHead(const std::string &key, int &value) const
    {
		auto iter = this->mHttpData.head().find(key);
		if(iter != this->mHttpData.head().end())
		{
			value = std::stoi(iter->second);
			return true;
		}
		return false;
    }

    bool HttpHandlerRequest::GetHead(const std::string &key, std::string &value) const
    {
		auto iter = this->mHttpData.head().find(key);
		if(iter != this->mHttpData.head().end())
		{
			value = iter->second;
			return true;
		}
		return false;
    }

    HttpStatus HttpHandlerRequest::OnReceiveData(asio::streambuf &streamBuffer)
    {
        std::iostream io(&streamBuffer);
        if(this->mState == HttpDecodeState::FirstLine)
        {
			std::string method, version;
            this->mState = HttpDecodeState::HeadLine;
            io >> method >> this->mUrl >> version;

			this->mHttpData.set_method(method);
			this->mHttpData.set_version(version);

			io.ignore(2); //去掉\r\n
        }
        if(this->mState == HttpDecodeState::HeadLine)
		{
			std::string lineData;
			while (std::getline(io, lineData))
			{
				if (lineData == "\r")
				{
					this->mState = HttpDecodeState::Content;
					break;
				}
				size_t pos = lineData.find(":");
				if (pos != std::string::npos)
				{
					size_t length = lineData.size() - pos - 2;
					std::string key = lineData.substr(0, pos);

					Helper::String::Tolower(key);
					std::string val = lineData.substr(pos + 1, length);
					this->mHttpData.mutable_head()->insert({key, val});
				}
			}
			if (this->mState == HttpDecodeState::Content)
			{
				if (this->GetMethod() == "GET")
				{
					size_t pos = this->mUrl.find("?");
					if (pos != std::string::npos)
					{
						this->mHttpData.set_data(this->mUrl.substr(pos + 1));
						this->mHttpData.set_path(this->mUrl.substr(0, pos));
					}
					return HttpStatus::OK;
				}
				else if (this->GetMethod() != "POST")
				{
					return HttpStatus::BAD_REQUEST;
				}
				this->mHttpData.set_path(this->mUrl);
			}
		}
        if(this->mState == HttpDecodeState::Content)
        {
            char buffer[256] = { 0 };
            size_t size = io.readsome(buffer, 256);
            while(size > 0)
            {
                this->mHttpData.mutable_data()->append(buffer, size);
                size = io.readsome(buffer, 256);
            }
        }
        return HttpStatus::CONTINUE;
    }
}

namespace Sentry
{
	HttpHandlerResponse::HttpHandlerResponse()
	{
		this->mCount = 0;
		this->mContentSize = 0;
		this->mFstream = nullptr;
		this->mCode = HttpStatus::OK;
	}

	HttpHandlerResponse::~HttpHandlerResponse()
	{
		if(this->mFstream != nullptr)
		{
			this->mFstream->close();
			delete this->mFstream;
		}
	}

	int HttpHandlerResponse::Serailize(std::ostream& os)
	{
		if(this->mCount == 0)
		{
			if(this->mFstream == nullptr)
			{
				this->mContentSize = this->mContent.size();
			}
			this->AddHead("content-length", std::to_string(this->mContentSize));
			os << HttpVersion << ' ' << (int)this->mCode << ' ' << HttpStatusToString(this->mCode) << "\r\n";
			for (auto iter = this->mHeadMap.begin(); iter != this->mHeadMap.end(); iter++)
			{
				const std::string& key = iter->first;
				const std::string& value = iter->second;
				os << key << ":" << value << "\r\n";
			}
			os << "\r\n";
		}
		this->mCount++;
		if(this->mFstream != nullptr)
		{
			char buffer[512] = { 0 };
			this->mFstream->read(buffer, 512);
			if(this->mFstream->gcount() > 0)
			{
				os.write(buffer, this->mFstream->gcount());
				return this->mContentSize - this->mFstream->gcount();
			}
			return 0;
		}
		os.write(this->mContent.c_str(), this->mContent.size());
		return 0;
	}

    bool HttpHandlerResponse::AddHead(const char *key, int value)
    {
        return this->AddHead(key, std::to_string(value));
    }

	bool HttpHandlerResponse::AddHead(const char * key, const std::string& value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, value);
		return true;
	}

	void HttpHandlerResponse::SetCode(HttpStatus code)
	{
		this->mCode = code;
	}

	void HttpHandlerResponse::WriteString(const std::string& content)
    {
        this->mContentSize += content.size();
        this->mContent.append(content);
    }

    void HttpHandlerResponse::WriteString(const char *content, size_t size)
    {
        this->mContentSize += size;
        this->mContent.append(content, size);
    }

	void HttpHandlerResponse::WriteFile(std::fstream * ofstream)
	{
		this->mFstream = ofstream;
		this->mFstream->seekg(0, this->mFstream->end);
		this->mContentSize = ofstream->tellg();
		this->mFstream->seekp(0, this->mFstream->beg);
	}
}
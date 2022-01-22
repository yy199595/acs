//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpAsyncRequest.h"
#include<regex>
#include"Http.h"
#include<iostream>
#include<spdlog/fmt/fmt.h>
namespace Sentry
{
    bool HttpAsyncRequest::ParseUrl(const std::string &url)
    {
        std::cmatch what;
        std::string protocol;
        std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
        if (std::regex_match(url.c_str(), what, pattern)) {
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

    bool HttpAsyncRequest::Get(const std::string &url)
    {
        if(!this->ParseUrl(url))
        {
            return false;
        }
        std::ostream os(&this->mSendStream);
        os << "GET " << this->mPath << " " << HttpVersion << "\r\n";
        os << "Host: " << this->mHost << "\r\n";
        os << "Accept: */*\r\n";
        os << "Connection: close\r\n\r\n";
        return true;
    }

    bool HttpAsyncRequest::Post(const std::string &url, const std::string &content)
    {
        if(!this->ParseUrl(url))
        {
            return false;
        }
        std::ostream os(&this->mSendStream);
        os << "POST " << this->mPath << " " << HttpVersion << "\r\n";
        os << "Host: " << this->mHost << ":" << this->mPort << "\r\n";
        os << "Content-Type: text/plain; charset=utf-8" << "\r\n";
        os << "Content-Length: " << content.size()<< "\r\n";
        os << "Accept: */*\r\n";
        os << "Connection: close\r\n\r\n";
        os.write(content.c_str(), content.size());
        return true;
    }

}

namespace Sentry
{
    HttpAsyncResponse::HttpAsyncResponse()
    {
        this->mHttpCode = 0;
        this->mContentLength = 0;
        this->mState = HttpDecodeState::FirstLine;
    }
    HttpStatus HttpAsyncResponse::OnReceiveData(asio::streambuf &streamBuffer)
    {
        std::iostream io(&streamBuffer);
        if(this->mState == HttpDecodeState::FirstLine)
        {
            this->mState = HttpDecodeState::HeadLine;
            io >> this->mVersion >> this->mHttpCode >> this->mHttpError;
            io.ignore(2); //放弃\r\n
        }
        if(this->mState == HttpDecodeState::HeadLine)
        {
            std::string lineData;
            while(std::getline(io, lineData))
            {
                if(lineData == "\r")
                {
                    this->mState = HttpDecodeState::Content;
                    break;
                }
                size_t pos = lineData.find(':');
                if (pos != std::string::npos)
                {
                    size_t length = lineData.size() - pos - 2;
                    std::string key = lineData.substr(0, pos);
                    std::string val = lineData.substr(pos + 1, length);
                    this->mHeadMap.insert(std::make_pair(key, val));
                }
            }
            if(this->mState == HttpDecodeState::Content)
            {
                auto iter = this->mHeadMap.find("Content-Length");
                if (iter == this->mHeadMap.end())
                {
                    return HttpStatus::LENGTH_REQUIRED;
                }
                const std::string &str = iter->second;
                this->mContentLength = std::stol(str);
            }
        }
        if(this->mState == HttpDecodeState::Content)
        {
            char buffer[256] = { 0 };
            size_t size = io.readsome(buffer, 256);
            while(size > 0)
            {
                this->mContent.append(buffer, size);
                if(this->mContent.size() == this->mContentLength)
                {
                    this->mState = HttpDecodeState::Finish;
                    return HttpStatus::OK;
                }
                size = io.readsome(buffer, 256);
            }
        }
        return HttpStatus::CONTINUE;
    }
}

namespace Sentry
{
    HttpHandlerRequest::HttpHandlerRequest()
    {
        this->mContentLength = 0;
        this->mState = HttpDecodeState::FirstLine;
    }

    bool HttpHandlerRequest::GetHeadContent(const std::string &key, std::string &value)
    {
        auto iter = this->mHeadMap.find(key);
        if(iter != this->mHeadMap.end())
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
            this->mState = HttpDecodeState::HeadLine;
            io >> this->mMethod >> this->mUrl >> this->mVersion;
            io.ignore(2); //去掉\r\n
        }
        if(this->mState == HttpDecodeState::HeadLine)
        {
            std::string lineData;
            while(std::getline(io, lineData))
            {
                if(lineData == "\r")
                {
                    this->mState = HttpDecodeState::Content;
                    break;
                }
                size_t pos = lineData.find(":");
                if (pos != std::string::npos)
                {
                    size_t length = lineData.size() - pos - 2;
                    std::string key = lineData.substr(0, pos);
                    std::string val = lineData.substr(pos + 1, length);
                    this->mHeadMap.insert(std::make_pair(key, val));
                }
            }
            if(this->mState == HttpDecodeState::Content)
            {
                if(this->mMethod == "GET")
                {
                    size_t pos = this->mUrl.find("?");
                    if(pos != std::string::npos)
                    {
                        this->mContent = mUrl.substr(pos + 1);
                        this->mUrl = this->mUrl.substr(0, pos);
                        return HttpStatus::OK;
                    }
                }
                else if(this->mMethod == "POST")
                {
                    auto iter = this->mHeadMap.find("Content-Length");
                    if (iter == this->mHeadMap.end())
                    {
                        return HttpStatus::LENGTH_REQUIRED;
                    }
                    const std::string &str = iter->second;
                    this->mContentLength = std::stol(str);
                    if(this->mContentLength <= 0)
                    {
                        return HttpStatus::LENGTH_REQUIRED;
                    }
                }
                else
                {
                    return HttpStatus::METHOD_NOT_ALLOWED;
                }
            }
        }
        if(this->mState == HttpDecodeState::Content)
        {
            if(this->mContentLength == 0)
            {
                return HttpStatus::OK;
            }
            char buffer[256] = { 0 };
            size_t size = io.readsome(buffer, 256);
            while(size > 0)
            {
                this->mContent.append(buffer, size);
                if(this->mContent.size() == this->mContentLength)
                {
                    this->mState = HttpDecodeState::Finish;
                    return HttpStatus::OK;
                }
                size = io.readsome(buffer, 256);
            }
        }
        return HttpStatus::CONTINUE;
    }
}

namespace Sentry
{
    void HttpHandlerResponse::AddValue(HttpStatus status)
    {
        std::ostream os(&this->mStreamBuffer);
        os << HttpVersion << (int)status << HttpStatusToString(status) << "\r\n";
        os << "\r\n";
    }

    void HttpHandlerResponse::AddValue(HttpStatus status, const std::string &content)
    {
        std::ostream os(&this->mStreamBuffer);
        os << HttpVersion << ' ' << (int)status << ' ' << HttpStatusToString(status) << "\r\n";
        os << "Content-Type: text/plain; charset=utf-8" << "\r\n";
        os << "Content-Length: " << content.size()<< "\r\n";
        os << "\r\n";
        os.write(content.c_str(), content.size());
    }
}
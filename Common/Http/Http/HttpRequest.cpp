//
// Created by yjz on 2022/10/27.
//

#include"HttpRequest.h"
#include<regex>
#include"String/StringHelper.h"


namespace Http
{
    Request::Request(const char *method)
        : mMethod(method)
    {
        this->mState = State::None;
        this->mVersion = HttpVersion;
    }

    bool Request::SetUrl(const std::string &url)
    {
        this->mUrl = url;
        std::cmatch what;
        std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
        if (std::regex_match(url.c_str(), what, pattern))
        {
            this->mHost = std::string(what[2].first, what[2].second);
            this->mPath = std::string(what[4].first, what[4].second);
            this->mProtocol.append(what[1].first, what[1].second);
            this->mPort = std::string(what[3].first, what[3].second);

            if (0 == this->mPort.length()) {
                this->mPort = "http" == this->mProtocol ? "80" : "443";
            }
            return true;
        }
        return false;
    }
    int Request::OnRead(std::istream &buffer)
    {
        if(this->mState == State::None)
        {
            buffer >> this->mUrl;
            buffer >> this->mVersion;
            this->mState = State::Head;
        }
        if(this->mState == State::Head)
        {
            if(this->mHead.OnRead(buffer) != 0)
            {
                return -1;
            }
            this->mState = State::Body;
        }
        return this->OnReadContent(buffer);
    }

    int Request::Serailize(std::ostream &os)
    {
        return this->OnWrite(os);
    }

    int Request::OnWrite(std::ostream &os)
    {
        os << this->mMethod << " " << this->mPath << " " << this->mVersion << "\r\n";
        os << "Host:" << this->mHost << "\r\n";
        this->mHead.OnWrite(os);
        os << "Connection: close\r\n\r\n";
        return this->OnWriteContent(os);
    }
}

namespace Http
{
    int GetRequest::OnReadContent(std::istream &buffer)
    {
        size_t pos = this->mUrl.find("?");
        if (pos != std::string::npos)
        {
            std::vector<std::string> result;
            std::vector<std::string> content;
            this->mPath = this->mUrl.substr(0, pos);
            std::string parameter = this->mUrl.substr(pos + 1);
            if(Helper::String::Split(parameter, "&", result) > 0)
            {
                for (const std::string &filed: result)
                {
                    content.clear();
                    if(Helper::String::Split(filed, "=", content) == 2)
                    {
                        const std::string & key = content[0];
                        const std::string & val = content[1];
                        this->mParameters.emplace(key, val);
                    }
                }
            }
            return 0;
        }
        this->mPath = this->mUrl;
        return 0;
    }

    bool GetRequest::GetParameter(const std::string & key, std::string & value)
    {
        auto iter = this->mParameters.find(key);
        if(iter == this->mParameters.end())
        {
            return false;
        }
        value = iter->second;
        return true;
    }

    int GetRequest::OnWriteContent(std::ostream &buffer)
    {
        return 0;
    }
}

namespace Http
{
    int PostRequest::OnReadContent(std::istream &buffer)
    {
        char buff[128] = {0};
        size_t size = buffer.readsome(buff, sizeof(buff));
        while(size > 0)
        {
            this->mContent.append(buff, size);
            size = buffer.readsome(buff, sizeof(buff));
        }
        return -1;
    }

    int PostRequest::OnWriteContent(std::ostream &buffer)
    {
        buffer.write(this->mContent.c_str(), this->mContent.size());
        return 0;
    }
}
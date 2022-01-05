//
// Created by zmhy0073 on 2021/12/16.
//

#include"HttpRespTask.h"
#include"Core/App.h"
namespace GameKeeper
{
    void HttpRespTask::OnReceiveHead(asio::streambuf &buf)
    {
        int code = 0;
        std::istream is(&buf);
        is >> this->mVersion >> code >> this->mError;
        this->mHttpCode = (HttpStatus)code;

        std::string line;
        std::getline(is, line);

        while (std::getline(is, line))
        {
            if (line == "\r")
            {
                break;
            }
            size_t pos = line.find(':');
            if (pos != std::string::npos)
            {
                size_t length = line.size() - pos - 2;
                std::string key = line.substr(0, pos);
                std::string val = line.substr(pos + 1, length);
                this->mHeadMap.insert(std::make_pair(key, val));
            }
        }
        std::string length;
        this->mContentLength = 0;
        if(this->GetHead("Content-Length", length))
        {
            this->mContentLength = std::stoi(length);
        }
    }

    void HttpRespTask::OnComplete(XCode code)
    {
        this->mCode = code;
        if(this->mHttpCode != HttpStatus::OK)
        {
            LOG_ERROR(this->mError);
        }
        this->SetResult(this->mResponse);
        auto iter = this->mHeadMap.begin();
        for(; iter != this->mHeadMap.end(); iter++)
        {
            LOG_WARN(iter->first << " " << iter->second);
        }
        LOG_WARN("content-length " << this->mResponse.size());
    }

    HttpStatus HttpRespTask::AwaitGetCode()
    {
        return this->mHttpCode;
    }

    bool HttpRespTask::OnReceiveBody(asio::streambuf & message)
    {
        char buffer[256] = { 0};
        std::istream is(&message);
        while(message.size() > 0)
        {
            size_t size = is.readsome(buffer, 256);
            for(size_t index = 0; index < size; index++)
            {
                if (buffer[index] != '\n' && buffer[index] != ' ')
                {
                    this->mResponse += buffer[index];
                }
            }
            //this->mResponse.append(buffer, size);
        }
        return this->mResponse.size() < this->mContentLength;
    }

    bool HttpRespTask::GetHead(const std::string &key, std::string &value) const
    {
        auto iter = this->mHeadMap.find(key);
        if(iter == this->mHeadMap.end())
        {
            return false;
        }
        value = iter->second;
        return true;
    }
}
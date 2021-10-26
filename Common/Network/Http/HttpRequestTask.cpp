//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRequestTask.h"
#include <Core/App.h>
#include <Util/StringHelper.h>
#include <Network/NetworkHelper.h>
#include <Util/TimeHelper.h>
namespace Sentry
{
    HttpRequestTask::HttpRequestTask(const std::string &url)
        : mUrl(url)
    {
        this->mReadCount = 0;
        this->mIsReadBody = false;
    }


    bool HttpRequestTask::Run()
    {
        unsigned short port;
        if (!NetworkHelper::ParseHttpUrl(this->mUrl, this->mHost, port, this->mPath))
        {
            this->mCode = XCode::HttpUrlParseError;
            return true;
        }
        asio::io_context io;
        asio::error_code err;
        asio::ip::tcp::resolver resolver(io);
        asio::ip::tcp::resolver::query query(this->mHost, std::to_string(port));
        auto iterator = resolver.resolve(query, err);
        if (err)
        {
            SayNoDebugError(err.message());
            this->mCode = XCode::HostResolverError;
            return true;
        }

        asio::ip::tcp::socket socket(io);
        asio::connect(socket, iterator, err);
        if (err)
        {
            SayNoDebugError(err.message());
            this->mCode = XCode::HttpNetWorkError;
            return true;
        }
        asio::streambuf httpStreamBuf;
        this->GetSendData(httpStreamBuf);

        asio::write(socket, httpStreamBuf, err);

        if (err)
        {
            SayNoDebugError(err.message());
            this->mCode = XCode::HttpNetWorkError;
            return true;
        }

        while(asio::read(socket, httpStreamBuf, asio::transfer_at_least(1), err))
        {
            SayNoDebugError(httpStreamBuf.size());
            if (err == asio::error::eof)
            {
                this->mCode = XCode::Successful;
                return true;
            }
            else if(err)
            {
                SayNoDebugError(err.message());
                this->mCode = XCode::HttpNetWorkError;
                return true;
            }
            if(this->mReadCount == 0)
            {
                this->mReadCount ++;
                std::istream is(&httpStreamBuf);
                is >> this->mVersion >> this->mHttpCode >> this->mError;
            }
            if(!this->mIsReadBody)
            {
                const char *data = asio::buffer_cast<const char *>(httpStreamBuf.data());
                const char *pos = strstr(data, "\r\n\r\n");
                if(pos == nullptr)
                {
                    continue;
                }
                size_t size = pos - data + strlen("\r\n\r\n");
                if (size != 0)
                {
                    this->mIsReadBody = true;
                    std::istream(&httpStreamBuf).ignore(size);
                    this->ParseHeard(std::string(data, size));
                }
            }
            if(this->mIsReadBody)
            {
                this->OnReceiveBody(httpStreamBuf);
            }
        }

        socket.close(err);
        this->mCode = XCode::Successful;
        return true;
    }

    void HttpRequestTask::ParseHeard(const std::string &heard)
    {
        std::vector<std::string> tempArray1;
        std::vector<std::string> tempArray2;
        StringHelper::SplitString(heard, "\n", tempArray1);
        for(const std::string & line : tempArray1)
        {
            StringHelper::SplitString(line, ":", tempArray2);
            if(tempArray2.size() == 2)
            {
                const std::string & key = tempArray2[0];
                const std::string & val = tempArray2[1];
                this->mHeardMap.insert(std::make_pair(key, val));
            }
        }
    }

    bool HttpRequestTask::GetHeardData(const std::string &key, std::string &value)
    {
        auto iter = this->mHeardMap.find(key);
        if(iter != this->mHeardMap.end())
        {
            value = iter->second;
            return true;
        }
        return false;
    }

}
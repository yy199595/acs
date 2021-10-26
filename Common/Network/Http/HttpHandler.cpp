//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpHandler.h"
#include <vector>
#include <Util/StringHelper.h>

namespace Sentry
{
    HttpHandler::HttpHandler(HttpRemoteSession *session)
        : mHttpSession(session)
    {

    }

    HttpHandler::~HttpHandler()
    {
        if(this->mHttpSession)
        {
            delete this->mHttpSession;
        }
    }

    void HttpHandler::OnReceiveHeard(const std::string &path, const std::string &version, const std::string &heard)
    {
        this->mPath = path;
        this->mVersion = version;
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

    void HttpHandler::ResponseData(int code, std::string *data)
    {
        std::ostream os(&mResponseBuf);
        os << this->mVersion << " " << code << " OK" << "\r\n\r\n";
        os << *data;
    }

    bool HttpHandler::GetHeardData(const std::string &key, std::string &value)
    {
        auto iter = this->mHeardMap.find(key);
        if(iter != this->mHeardMap.end())
        {
            value = iter->second;
            return true;
        }
        return false;
    }

    bool HttpHandler::OnReceive(asio::streambuf &streambuf, const asio::error_code &err)
    {
        if(err == asio::error::eof)
        {
            this->OnReceiveDone();
            return false;
        }
        else if(err)
        {
            this->OnReceiveDone();
            return false;
        }
        this->OnReceiveBody(streambuf);
        return true;
    }
}
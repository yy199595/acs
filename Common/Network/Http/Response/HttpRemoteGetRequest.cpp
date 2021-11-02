//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpRemoteGetRequest.h"
#include <Util/StringHelper.h>
#include <Define/CommonDef.h>
namespace GameKeeper
{

    bool HttpRemoteGetRequest::OnReceiveBody(asio::streambuf &buf)
    {
		return false;
    }

    bool HttpRemoteGetRequest::GetParameter(const std::string &key, std::string &val)
    {
        auto iter = this->mParameMap.find(key);
        if (iter == this->mParameMap.end())
        {
            return false;
        }
        val = iter->second;
        return true;
    }

    bool HttpRemoteGetRequest::OnReceiveHeard(asio::streambuf &buf, size_t size)
    {
        std::string path;
        std::istream is(&buf);
        is >> path >> this->mVersion;
        std::vector<std::string> tempArray1;

        size_t pos = path.find('?');
        if(pos == std::string::npos)
        {
            this->mPath = path;
            return true;
        }

        std::vector<std::string> tempArray2;
        this->mPath = path.substr(0, pos);
        std::string parameter = path.substr(pos + 1);

        StringHelper::SplitString(parameter, "&", tempArray1);

        for(const std::string & data : tempArray1)
        {
            StringHelper::SplitString(data, "=", tempArray2);
            if(tempArray2.size() == 2)
            {
                const std::string &key = tempArray2[0];
                const std::string &val = tempArray2[1];
                this->mParameMap.insert(std::make_pair(key, val));
                GKDebugError("get parameter " << key << " = " << val);
            }
        }
        this->ParseHeard(buf, size - (size - buf.size()));
        this->NoticeMainThread();
        return false;
    }
}
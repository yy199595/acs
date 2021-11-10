//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpGettHandler.h"
#include <Util/StringHelper.h>
#include <Define/CommonDef.h>
#include <Core/App.h>
#include <Scene/RpcProtoComponent.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpComponent.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{

//	bool HttpGettHandler::SplitParameter(std::unordered_map<std::string, std::string>& parames)
//	{
//		parames.clear();
//		std::vector<std::string> tempArray1;
//		std::vector<std::string> tempArray2;
//		StringHelper::SplitString(this->mParamater, "&", tempArray1);
//		for (const std::string & data : tempArray1)
//		{
//			StringHelper::SplitString(data, "=", tempArray2);
//			if (tempArray2.size() != 2)
//			{
//				return false;
//			}
//			const std::string &key = tempArray2[0];
//			const std::string &val = tempArray2[1];
//			parames.insert(std::make_pair(key, val));
//			GKDebugError("get parameter " << key << " = " << val);
//		}
//		return true;
//	}

    void HttpGettHandler::Clear()
    {
        this->mPath.clear();
        this->mVersion.clear();
        this->mParamater.clear();
        HttpRequestHandler::Clear();
    }

	bool HttpGettHandler::OnReceiveHead(asio::streambuf & streamBuf)
    {
        std::string url;
        std::istream is(&streamBuf);
        is >> url >> this->mVersion;
        this->ParseHeard(streamBuf);
        const std::string & app = "App/";

        size_t pos1 = url.find(app) + app.length();
        GKAssertRetFalse_F(pos1 != std::string::npos);
        size_t pos2 = url.find('/', pos1 + 1);
        GKAssertRetFalse_F(pos2 != std::string::npos);

        size_t pos3 = url.find('/', pos2 + 1);
        if(pos3 == std::string::npos)
        {
            pos3 = url.find('?', pos2 + 1);
        }
        if(pos3 != std::string::npos)
        {
            this->mParamater = url.substr(pos3 + 1);
        }
        this->mComponent = url.substr(pos1, pos2 - pos1);
        this->mMethod = url.substr(pos2 + 1, pos3 - pos2 - 1);
        GKDebugLog("[http GET] " << this->mComponent << "." << this->mMethod << " data " << this->mParamater);
        return true;
    }

    size_t HttpGettHandler::ReadFromStream(std::string & stringBuf)
    {
        if(!this->mParamater.empty())
        {
            stringBuf.append(this->mParamater);
            this->mParamater.clear();
            return stringBuf.size();
        }
        return 0;
    }
}
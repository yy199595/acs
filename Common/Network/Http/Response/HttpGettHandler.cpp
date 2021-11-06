//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpGettHandler.h"
#include <Util/StringHelper.h>
#include <Define/CommonDef.h>
#include <Core/App.h>
#include <Scene/ProtocolComponent.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpClientComponent.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{

	bool HttpGettHandler::SplitParameter(std::unordered_map<std::string, std::string>& parames)
	{
		parames.clear();
		std::vector<std::string> tempArray1;
		std::vector<std::string> tempArray2;
		StringHelper::SplitString(this->mParamater, "&", tempArray1);
		for (const std::string & data : tempArray1)
		{
			StringHelper::SplitString(data, "=", tempArray2);
			if (tempArray2.size() != 2)
			{
				return false;
			}
			const std::string &key = tempArray2[0];
			const std::string &val = tempArray2[1];
			parames.insert(std::make_pair(key, val));
			GKDebugError("get parameter " << key << " = " << val);
		}
		return true;
	}

	void HttpGettHandler::OnReceiveHeardAfter(XCode code)
    {
        this->mCode = code;
		GKDebugInfo(this->PrintHeard());
        auto protocolComponent = App::Get().GetComponent<ProtocolComponent>();
        this->mHttpConfig = protocolComponent->GetHttpConfig(this->mPath);
        if (this->mHttpConfig == nullptr)
        {
            this->SetCode(HttpStatus::NOT_FOUND);
            return;
        }
        const std::string &method = this->mHttpConfig->Method;
        const std::string &service = this->mHttpConfig->Service;
        auto httpMethod = this->mHttpComponent->GetHttpMethod(service, method);
        if (httpMethod == nullptr)
        {
            this->SetCode(HttpStatus::NOT_FOUND);
            return;
        }
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&HttpClientComponent::HandlerHttpRequest, this->mHttpComponent, this);
    }

	bool HttpGettHandler::OnReceiveHeard(asio::streambuf & streamBuf)
    {
		std::string url;
		std::istream is(&streamBuf);
		is >> url >> this->mVersion;
		this->ParseHeard(streamBuf);

        size_t pos = url.find('?');
        if (pos != std::string::npos)
        {
            this->mPath = url.substr(0, pos);
            this->mParamater = url.substr(pos + 1);
            GKDebugLog(this->mPath);
            GKDebugInfo(this->mParamater);
            return false;
        }
        this->mPath = url;
        return false;
    }
}
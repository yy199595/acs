//
// Created by zmhy0073 on 2021/11/4.
//

#include "HttpPostHandler.h"
#include <Core/App.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpComponent.h>
#include <Scene/RpcProtoComponent.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    HttpPostHandler::HttpPostHandler(HttpComponent *component)
        : HttpRequestHandler(component)
    {

    }

    const std::string &HttpPostHandler::GetPath()
    {
		return this->mPath;
    }

    void HttpPostHandler::Clear()
    {
        this->mPath.clear();
        this->mVersion.clear();
        this->mParamater.clear();
        HttpRequestHandler::Clear();
    }

	bool HttpPostHandler::OnReceiveHead(asio::streambuf &streamBuf)
	{
		std::istream is(&streamBuf);
		is >> this->mPath >> this->mVersion;
        this->ParseHeard(streamBuf);

        const std::string app = "App/";
        size_t pos1 = this->mPath.find(app) + app.length();
        size_t pos2 = this->mPath.find('/', pos1 + 1);
        GKAssertRetFalse_F(pos2 != std::string::npos);

        this->mMethod = this->mPath.substr(pos2 + 1);
        this->mComponent = this->mPath.substr(pos1, pos2 - pos1);
        GKDebugLog("[http POST]" << this->mComponent << "." << this->mMethod << " length " << this->GetContentLength());

        while(streamBuf.size() > 0)
        {
           size_t size = is.readsome(this->mHandlerBuffer, 1024);
           this->mParamater.append(this->mHandlerBuffer, size);
        }
		return true;
	}

    void HttpPostHandler::OnReceiveBody(asio::streambuf &streamBuf)
    {
        std::istream is(&streamBuf);
        while(streamBuf.size() > 0)
        {
            size_t size = is.readsome(this->mHandlerBuffer, 1024);
            this->mParamater.append(this->mHandlerBuffer, size);
        }
    }

    size_t HttpPostHandler::ReadFromStream(std::string & stringBuf)
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


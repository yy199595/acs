//
// Created by zmhy0073 on 2021/11/4.
//

#include "HttpPostHandler.h"
#include <Core/App.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpClientComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    HttpPostHandler::HttpPostHandler(HttpClientComponent *component, HttpRemoteSession *session)
                                                               : HttpRequestHandler(component, session)
    {
		this->mParamater.clear();
    }

    void HttpPostHandler::OnReceiveBodyAfter(XCode code)
    {
        this->mCode = code;
        if (this->mCode != XCode::Successful)
        {
            this->OnWriterAfter(code);
            return;
        }
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&HttpClientComponent::HandlerHttpRequest, this->mHttpComponent, this);
    }

    const std::string &HttpPostHandler::GetPath()
    {
		return this->mPath;
    }

	bool HttpPostHandler::OnReceiveHeard(asio::streambuf &streamBuf)
	{
		std::istream is(&streamBuf);
		is >> this->mPath >> this->mVersion;
		auto protocolComponent = App::Get().GetComponent<ProtocolComponent>();
		this->mHttpConfig = protocolComponent->GetHttpConfig(this->GetPath());
		if (this->mHttpConfig == nullptr)
		{
			this->SetCode(HttpStatus::NOT_FOUND);
			GKDebugError("not find url " << this->GetPath());
			return false;
		}
		const std::string &method = this->mHttpConfig->Method;
		const std::string &service = this->mHttpConfig->Service;
		if (!this->mHttpComponent->GetHttpMethod(service, method))
		{
			this->SetCode(HttpStatus::NOT_FOUND);
			GKDebugError("not find method " << service << "." << method);
			return false;
		}
		this->ParseHeard(streamBuf);

		this->mDataLength = this->GetContentLength();
		if (this->mDataLength == 0)
		{
			this->SetCode(HttpStatus::LENGTH_REQUIRED);
			return false;
		}
		/*std::string disposltion;
		if(this->GetHeardData("Content-Disposition", disposltion))
		{
			const std::string ss = "filename=";
			size_t pos = disposltion.find(ss);
			if (pos == std::string::npos)
			{
				this->SetCode(HttpStatus::PRECONDITION_FAILED);
				return false;
			}
			size_t offset = pos + ss.size();
			std::string file = disposltion.substr(offset, disposltion.size() - offset);
			const std::string path = App::Get().GetDownloadPath() + file;
			return true;
		}*/
		return true;
	}

    void HttpPostHandler::OnReceiveHeardAfter(XCode code)
    {
        this->mCode = code;
        if(this->mCode != XCode::Successful)
        {
            this->OnWriterAfter(code);
        }
		GKDebugInfo(this->PrintHeard());
    }


    void HttpPostHandler::OnReceiveBody(asio::streambuf &buf)
    {
        std::istream is(&buf);
        while(buf.size() > 0)
        {
            size_t size = is.readsome(this->mHandlerBuffer, 1024);
			this->mParamater.append(this->mHandlerBuffer, size);
        }
    }
}


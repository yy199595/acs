//
// Created by zmhy0073 on 2021/11/4.
//

#include "HttpPostHandler.h"
#include <Core/App.h>
#include <Network/Http/HttpRespSession.h>
#include "Http/Component/HttpClientComponent.h"
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    HttpPostHandler::HttpPostHandler(HttpClientComponent *component)
        : HttpRequestHandler(component)
    {
        this->mContent = nullptr;
    }

    const std::string &HttpPostHandler::GetPath()
    {
		return this->mPath;
    }

    void HttpPostHandler::Clear()
    {
        this->mPath.clear();
        this->mVersion.clear();
        delete this->mContent;
        this->mContent = nullptr;
        HttpRequestHandler::Clear();
    }

	bool HttpPostHandler::OnReceiveHead(asio::streambuf &streamBuf)
    {
        std::istream is(&streamBuf);
        is >> this->mPath >> this->mVersion;
        this->ParseHeard(streamBuf);

        std::string content;
        if (this->GetContentType(content))
        {
            if (content.find("text/plain") != std::string::npos ||
                content.find("applocation/json") != std::string::npos)
            {
                this->mContent = new HttpReadStringContent();
            }
            else if (content.find("multipart/form-data"))
            {
                std::string fullPath;
                if(!this->GetHeardData("Content-Path", fullPath))
                {
                    return false;
                }
                auto fileContent = new HttpReadFileContent(fullPath);
                if (!fileContent->OpenFile())
                {
                    delete fileContent;
                    LOG_ERROR("not open file {0}", fullPath);
                    return false;
                }
                this->mContent = fileContent;
            }
        }
        if (this->mContent == nullptr)
        {
            return false;
        }

        const std::string app = "App/";
        size_t pos1 = this->mPath.find(app);
        LOG_CHECK_RET_FALSE(pos1 != std::string::npos);
        pos1 = pos1 + app.length();
        size_t pos2 = this->mPath.find('/', pos1 + 1);
        LOG_CHECK_RET_FALSE(pos2 != std::string::npos);

        this->mMethod = this->mPath.substr(pos2 + 1);
        this->mComponent = this->mPath.substr(pos1, pos2 - pos1);
        LOG_DEBUG("[http POST] {0}.{1} legth {2}", this->mComponent,this->mMethod ,this->GetContentLength());

        char buffer[256] = { 0 };
        while (streamBuf.size() > 0)
        {
            size_t size = is.readsome(buffer, 256);
            this->mContent->OnReadContent(buffer, size);
        }
#ifdef __DEBUG__
        std::stringstream sss;
        sss << "\n========== http post ==========\n";
        sss << this->PrintHeard();
        sss << "==================================";
        LOG_INFO(sss.str());
#endif
        return true;
    }

    bool HttpPostHandler::OnReceiveBody(asio::streambuf &streamBuf)
    {
        char buffer[256] = { 0 };
        std::istream is(&streamBuf);
        while(streamBuf.size() > 0)
        {
            size_t size = is.readsome(buffer, 256);
            this->mContent->OnReadContent(buffer, size);
        }
        if(dynamic_cast<HttpReadStringContent *>(this->mContent))
        {
            if (this->mContent->GetContentSize() >= HttpPostMaxCount)
            {
                LOG_ERROR("http post data failure");
                return false;
            }
        }
        return this->mContent->GetContentSize() == this->GetContentLength();
    }
}


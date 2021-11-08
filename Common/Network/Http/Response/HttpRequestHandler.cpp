//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpRequestHandler.h"
#include <Core/App.h>
#include <Other/ProtocolConfig.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpClientComponent.h>
#include <Network/Http/Content/HttpReadContent.h>
#include <Network/Http/Content/HttpWriteContent.h>
namespace GameKeeper
{

    HttpRequestHandler::HttpRequestHandler(HttpClientComponent *component)
        : mHttpComponent(component)
    {
        this->mWriteCount = 0;
        this->mHttpConfig = nullptr;
        this->mResponseContent = nullptr;
#ifdef __DEBUG__
        this->mStartTime = TimeHelper::GetMilTimestamp();
#endif
    }

    HttpRequestHandler::~HttpRequestHandler() noexcept
    {
        delete this->mResponseContent;
#ifdef __DEBUG__
        if(this->mHttpConfig != nullptr)
        {
            long long endTime = TimeHelper::GetMilTimestamp();
            GKDebugLog("http call " << this->mHttpConfig->Service << "." << this->mHttpConfig->Method
                                    << " use time = " << ((endTime - this->mStartTime) / 1000.0f) << "s");
        }
#endif
    }

    void HttpRequestHandler::SetResponseCode(HttpStatus code)
    {
        this->mHttpCode = code;
    }

    bool HttpRequestHandler::WriterToBuffer(std::ostream &os)
    {
        if (this->mWriteCount == 0)
        {
            os << this->mVersion << " " << (int) this->mHttpCode
               << " " << HttpStatusToString(this->mHttpCode) << "\r\n";
            if (this->mResponseContent != nullptr)
            {
                this->mResponseContent->GetContentType(os);
                os << "Content-Length:" << this->mResponseContent->GetContentSize() << "\r\n";
            }
            os << "Server:" << "GameKeeper" << "\r\n";
            os << "Connection:close" << "\r\n";
            auto iter = this->mHeardMap.begin();
            for (; iter != this->mHeardMap.end(); iter++)
            {
                const std::string &key = iter->first;
                const std::string &val = iter->second;
                os << key << ":" << val << "\r\n";
            }
            os << "\r\n";
        }
        this->mWriteCount++;
        if (this->mResponseContent == nullptr)
        {
            return true;
        }
        return this->mResponseContent->GetContent(os);
    }

    bool HttpRequestHandler::AddResponseHeard(const std::string &key, const std::string &val)
    {
        auto iter = this->mHeardMap.find(key);
        if(iter == this->mHeardMap.end())
        {
            this->mHeardMap.emplace(key, val);
            return true;
        }
        return false;
    }

	void HttpRequestHandler::Clear()
	{
		HttpHandlerBase::Clear();
		this->mWriteCount = 0;
		this->mVersion.clear();
		this->mHeardMap.clear();
		this->mHttpConfig = nullptr;
		delete this->mResponseContent;
		this->mResponseContent = nullptr;
	}


    void HttpRequestHandler::SetResponseContent(HttpWriteContent *httpContent)
    {
        delete this->mResponseContent;
        this->mResponseContent = httpContent;
    }
}
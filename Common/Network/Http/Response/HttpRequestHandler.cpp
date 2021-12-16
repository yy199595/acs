//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpRequestHandler.h"
#include <Core/App.h>
#include <Other/ProtocolConfig.h>
#include <Network/Http/HttpRespSession.h>
#include <Component/Http/HttpComponent.h>
#include <Network/Http/Content/HttpReadContent.h>
#include <Network/Http/Content/HttpWriteContent.h>
namespace GameKeeper
{

    HttpRequestHandler::HttpRequestHandler(HttpComponent *component)
        : mHttpComponent(component)
    {
        this->mWriteCount = 0;
        //this->mHttpConfig = nullptr;
        this->mResponseContent = nullptr;
#ifdef __DEBUG__
        this->mStartTime = TimeHelper::GetMilTimestamp();
#endif
    }

    HttpRequestHandler::~HttpRequestHandler() noexcept
    {
        delete this->mResponseContent;
    }

    void HttpRequestHandler::SetResponseCode(HttpStatus code)
    {
        this->mHttpCode = code;
    }

    void HttpRequestHandler::WriterToBuffer(std::ostream &os)
    {

        HttpStatus code = this->mHttpCode;
        os << HttpVersion << " " << (int) code << " " << HttpStatusToString(code) << "\r\n";

        auto iter = this->mHeardMap.begin();
        for (; iter != this->mHeardMap.end(); iter++)
        {
            const std::string &key = iter->first;
            const std::string &val = iter->second;
            os << key << ": " << val << "\r\n";
        }
        if (this->mResponseContent != nullptr)
        {
            this->mResponseContent->WriteHead(os);
        }

        os << "Server: " << "GameKeeper" << "\r\n";
        os << "Connection: " << "close" << "\r\n\r\n";
        this->mWriteCount++;
        this->mResponseContent->WriteBody(os);
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
        this->mMethod.clear();
        this->mComponent.clear();
		delete this->mResponseContent;
		this->mResponseContent = nullptr;
	}


    void HttpRequestHandler::SetResponseContent(HttpWriteContent *httpContent)
    {
        delete this->mResponseContent;
        this->mResponseContent = httpContent;
    }
}
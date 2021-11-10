//
// Created by zmhy0073 on 2021/11/10.
//

#include "HttpUploadHandler.h"
#include <Core/App.h>
#include <Component/Scene/HttpComponent.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    const std::string &HttpUploadHandler::GetPath()
    {
        return this->mPath;
    }

    void HttpUploadHandler::Clear()
    {
        this->mPath.clear();
        this->mVersion.clear();
        this->mParamater.clear();
        HttpRequestHandler::Clear();
    }

    bool HttpUploadHandler::OnReceiveHead(asio::streambuf &streamBuf)
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

        std::string contentType;

        if(!this->GetHeardData("Content-Type", contentType))
        {
            return false;
        }
        size_t pos = contentType.find("multipart/form-data");
        if(pos == std::string::npos)
        {
            return false;
        }
        std::string contentDisposition;
        const std::string str = "filename:";
        if(!this->GetHeardData("Content-Disposition", contentDisposition))
        {
            return false;
        }
        pos = contentDisposition.find(str);
        if(pos == std::string::npos)
        {
            return false;
        }
        this->mWriteBytes = 0;
        const std::string & path = App::Get().GetDownloadPath();
        std::string filename = contentDisposition.substr(pos + str.size());
        this->mFstream.open(path + filename, std::ios::out | std::ios::binary);
        return this->mFstream.is_open();
    }

    bool HttpUploadHandler::OnReceiveBody(asio::streambuf &streamBuf)
    {
        std::istream is(&streamBuf);
        while(streamBuf.size() > 0 && this->mFstream.is_open())
        {
            size_t size = is.readsome(this->mHandlerBuffer, 1024);
            this->mFstream.write(this->mHandlerBuffer, size);
        }
        if(this->mWriteBytes == this->GetContentLength())
        {
            this->mFstream.close();
            return true;
        }
        return false;
    }

    size_t HttpUploadHandler::ReadFromStream(std::string & stringBuf)
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


//
// Created by zmhy0073 on 2021/11/3.
//

#include "HttpReadContent.h"
#include <Util/DirectoryHelper.h>
#include <Define/CommonDef.h>
namespace GameKeeper
{
    HttpReadStringContent::HttpReadStringContent(std::string &response)
        : mResponse(response)
    {

    }

    void HttpReadStringContent::OnReadContent(const char *data, size_t size)
    {
        this->mResponse.append(data, size);
    }
}

namespace GameKeeper
{
    HttpReadFileContent::HttpReadFileContent(const std::string &path)
        : mPath(path)
    {

    }

    bool HttpReadFileContent::OpenFile()
    {
        std::string dir;
        if (!DirectoryHelper::GetDirByPath(this->mPath, dir))
        {
            GKDebugError("parse " << this->mPath << " failure");
            return false;
        }
        GKDebugWarning(dir);
        if(!DirectoryHelper::MakeDir(dir))
        {
            GKDebugError("create dir " << dir << " failure");
            return false;
        }
        this->mFileStream.open(this->mPath);
        if(!this->mFileStream.is_open())
        {
            GKDebugError("open or create " << this->mPath << " failure");
            return false;
        }
        return true;
    }


    HttpReadFileContent::~HttpReadFileContent() noexcept
    {
        if(this->mFileStream.is_open())
        {
            this->mFileStream.close();
        }
    }

    void HttpReadFileContent::OnReadContent(const char *data, size_t size)
    {
        if(!this->mFileStream.is_open())
        {
            GKDebugError("writer file to " << this->mPath << " failure");
            return;
        }
        this->mFileStream.write(data, size);
        GKDebugWarning("writer to file " << size);
    }
}
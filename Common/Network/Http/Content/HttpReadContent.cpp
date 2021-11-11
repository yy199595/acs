//
// Created by zmhy0073 on 2021/11/3.
//

#include "HttpReadContent.h"
#include <Util/DirectoryHelper.h>
#include <Define/CommonDef.h>
namespace GameKeeper
{
    HttpReadStringContent::HttpReadStringContent(std::string &response)
        : mResponse(&response), mIsDelete(false)
    {

    }

    HttpReadStringContent::HttpReadStringContent()
            : mResponse(new std::string()), mIsDelete(true)
    {

    }
    HttpReadStringContent::~HttpReadStringContent() noexcept
    {
        if(this->mIsDelete)
        {
            delete this->mResponse;
        }
    }

    void HttpReadStringContent::OnReadContent(const char *data, size_t size)
    {
        this->mResponse->append(data, size);
    }
}

namespace GameKeeper
{
    HttpReadFileContent::HttpReadFileContent(const std::string &path)
        : mPath(path)
    {
        this->mFileSize=0;
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
        if (!DirectoryHelper::MakeDir(dir))
        {
            GKDebugError("create dir " << dir << " failure");
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
        if (!this->mFileStream.is_open())
        {
            this->mFileStream.open(this->mPath, std::ios::out | std::ios::binary);
        }
        if (!this->mFileStream.is_open())
        {
            GKDebugError("open or create " << this->mPath << " failure");
            return;
        }
        this->mFileSize += size;
        this->mFileStream.write(data, size);
    }
}
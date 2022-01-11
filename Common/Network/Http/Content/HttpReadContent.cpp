//
// Created by zmhy0073 on 2021/11/3.
//

#include "HttpReadContent.h"
#include"Core/App.h"
#include <Util/DirectoryHelper.h>
#include <Define/CommonLogDef.h>
namespace GameKeeper
{
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
        this->mFileSize=0;
    }

    bool HttpReadFileContent::OpenFile()
    {
        std::string dir;
        if (!Helper::Directory::GetDirByPath(this->mPath, dir))
        {
            LOG_ERROR("parse {0} failure", this->mPath);
            return false;
        }
        LOG_WARN(dir);
        if (!Helper::Directory::MakeDir(dir))
        {
            LOG_ERROR("create dir {0} failure", dir);
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
            LOG_ERROR("open or create {0} failure", this->mPath);
            return;
        }
        this->mFileSize += size;
        this->mFileStream.write(data, size);
    }
}
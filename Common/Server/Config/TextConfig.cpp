//
// Created by zmhy0073 on 2022/10/13.
//
#include"TextConfig.h"
#include"File/FileHelper.h"
#include"Md5/MD5.h"
#include"Log/CommonLogDef.h"
namespace Sentry
{
    bool TextConfig::LoadConfig(const std::string &path)
    {
        std::string content;
        this->mPath = path;
        if(!Helper::File::ReadTxtFile(this->mPath, content))
        {
            CONSOLE_LOG_ERROR("read file [" << path << "] error");
            return false;
        }
        if(this->OnLoadText(content))
        {
            this->mMd5 = Helper::Md5::GetMd5(content);
            return true;
        }
        return false;
    }

    bool TextConfig::ReloadConfig()
    {
        std::string content;
        if(!Helper::File::ReadTxtFile(this->mPath, content))
        {
            CONSOLE_LOG_ERROR("read file [" << this->mPath << "] error");
            return false;
        }

        std::string md5 = Helper::Md5::GetMd5(content);
        if(md5 == this->mMd5)
        {
            return true;
        }
        if(this->OnReloadText(content))
        {
            this->mMd5 = md5;
            return true;
        }
        return false;
    }
}
//
// Created by zmhy0073 on 2022/10/13.
//
#include"TextConfig.h"
#include"File/FileHelper.h"
#include"Md5/MD5.h"
#include"System/System.h"
#include"Log/CommonLogDef.h"
namespace Sentry
{
    const std::string &TextConfig::WorkPath() const
    {
        return System::WorkPath();
    }
    bool TextConfig::LoadConfig(const std::string &path)
    {
        std::string content;
        this->mPath = path;
        if(!Helper::File::ReadTxtFile(this->mPath, content))
        {
            CONSOLE_LOG_ERROR("read file [" << path << "] error");
            return false;
        }
        if(this->OnLoadText(content.c_str(), content.size()))
        {
            this->mMd5 = Helper::Md5::GetMd5(content);
            return true;
        }
        return false;
    }

    bool TextConfig::ReloadConfig()
    {
        std::string content, md5;
        if(!Helper::File::ReadTxtFile(this->mPath, content, md5))
        {
            CONSOLE_LOG_ERROR("read file [" << this->mPath << "] error");
            return false;
        }
        if(md5 == this->mMd5) //文件无变更
        {
            return true;
        }
        if(this->OnReloadText(content.c_str(), content.size()))
        {
            this->mMd5 = md5;
            return true;
        }
        return false;
    }
}
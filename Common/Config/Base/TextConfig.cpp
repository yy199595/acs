//
// Created by zmhy0073 on 2022/10/13.
//
#include"TextConfig.h"
#include"Util/File/FileHelper.h"
#include"Util/Md5/MD5.h"
#include"Core/System/System.h"
#include"Log/Common/CommonLogDef.h"
namespace Tendo
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
            CONSOLE_LOG_ERROR("load file [" << path << "] error");
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
        std::string content;
		if(!Helper::File::ReadTxtFile(this->mPath, content))
		{
			CONSOLE_LOG_ERROR("reload file [" << this->mPath << "] error");
			return false;
		}
		std::string md5 = Helper::Md5::GetMd5(content);
		if(md5 == this->mMd5 || this->OnReloadText(content.c_str(), content.size()))
		{
			this->mMd5 = md5;
			return true;
		}
        return false;
    }
}
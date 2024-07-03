//
// Created by zmhy0073 on 2022/10/13.
//
#include"TextConfig.h"
#include"Util/File/FileHelper.h"
#include"Core/System/System.h"
#include"Log/Common/CommonLogDef.h"
namespace joke
{
    bool TextConfig::LoadConfig(const std::string &path)
    {
        std::string content;
        this->mPath = path;
		if(!System::ReadFile(this->mPath, content))
		{
			CONSOLE_LOG_ERROR("read file [ {} ] fail", this->mPath);
			return false;
		}
        if(this->OnLoadText(content.c_str(), content.size()))
        {
            this->mLastWriteTime = help::fs::GetLastWriteTime(path);
            return true;
        }
        return false;
    }

    bool TextConfig::ReloadConfig()
    {
		long long t1 = help::fs::GetLastWriteTime(this->mPath);
		if(t1 == this->mLastWriteTime)
		{
			return true;
		}
        std::string content;
		this->mLastWriteTime = t1;
		if(!help::fs::ReadTxtFile(this->mPath, content))
		{
			CONSOLE_LOG_ERROR("reload file error", this->mPath);
			return false;
		}
		return this->OnReloadText(content.c_str(), content.size());
    }
}
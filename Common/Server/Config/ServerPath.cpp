//
// Created by zmhy0073 on 2021/10/26.
//

#include"ServerPath.h"
#include"File/DirectoryHelper.h"
namespace Sentry
{
    ServerPath::ServerPath(int argc, char **argv)
    {
        Helper::Directory::GetDirByPath(argv[0], this->mWorkPath);

        this->mLogsPath = this->mWorkPath + "Logs/";
        this->mConfigPath = this->mWorkPath + "config/";
        this->mDownloadPath = this->mWorkPath + "Download/";
    }
}
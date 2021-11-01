//
// Created by zmhy0073 on 2021/10/26.
//

#include"ServerPath.h"
#include<Util/DirectoryHelper.h>
namespace GameKeeper
{
    ServerPath::ServerPath(int argc, char **argv)
    {
        DirectoryHelper::GetDirByPath(argv[0], this->mWorkPath);

        this->mLogsPath = this->mWorkPath + "Logs/";
        this->mConfigPath = this->mWorkPath + "Config/";
    }
}
//
// Created by zmhy0073 on 2022/1/8.
//

#include"LuaTaskSource.h"
#include"Pool/MessagePool.h"
namespace GameKeeper
{
    XCode LuaTaskSource::Await()
    {
        return this->mTaskSource.Await();
    }

    void LuaTaskSource::SetResult(int result, std::string json)
    {
        XCode code = (XCode)code;
        this->mJson = std::move(json);
        this->mTaskSource.SetResult(code);
    }
}
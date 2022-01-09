//
// Created by zmhy0073 on 2022/1/8.
//

#include"LuaTaskSource.h"
#include"Pool/MessagePool.h"
namespace GameKeeper
{
    LuaTaskSource::LuaTaskSource()
    {
        this->mCode = XCode::LuaCoroutineWait;
    }
    XCode LuaTaskSource::Await()
    {
        if(this->mCode == XCode::LuaCoroutineWait) {
            return this->mTaskSource.Await();
        }
        return this->mCode;
    }

    void LuaTaskSource::SetResult(int result, std::string & json)
    {
        this->mCode = (XCode) result;
        this->mJson = std::move(json);
        this->mTaskSource.SetResult(this->mCode);
    }
}
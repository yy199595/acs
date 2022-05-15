//
// Created by zmhy0073 on 2022/1/8.
//

#include"LuaServiceTaskSource.h"
#include"Pool/MessagePool.h"

namespace Sentry
{
    LuaServiceTaskSource::LuaServiceTaskSource()
    {
        this->mCode = XCode::LuaCoroutineWait;
    }
    XCode LuaServiceTaskSource::Await()
    {
        if(this->mCode == XCode::LuaCoroutineWait) {
            return this->mTaskSource.Await();
        }
        return this->mCode;
    }

	void LuaServiceTaskSource::SetError(const std::string& error)
	{
		this->mCode = XCode::CallLuaFunctionFail;
		this->mJson = std::move(error);
		this->mTaskSource.SetResult(this->mCode);
	}

    void LuaServiceTaskSource::SetResult(int result, std::string & json)
    {
        this->mCode = (XCode) result;
        this->mJson = std::move(json);
        this->mTaskSource.SetResult(this->mCode);
    }
}
//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef GAMEKEEPER_LUATASKSOURCE_H
#define GAMEKEEPER_LUATASKSOURCE_H
#include"TaskSource.h"

namespace Sentry
{
    class LuaServiceTaskSource final
    {
    public:
        LuaServiceTaskSource(lua_State * lua);
        ~LuaServiceTaskSource();
    public:
		static int SetResult(lua_State * lua);
    public:
        XCode Await();
		int GetTable() { return this->mRef;}
    private:
		int mRef;
        XCode mCode;
		lua_State * mLua;
        TaskSource<XCode> mTaskSource;
    };
}
#endif //GAMEKEEPER_LUATASKSOURCE_H

//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef GAMEKEEPER_LUATASKSOURCE_H
#define GAMEKEEPER_LUATASKSOURCE_H
#include"Source/TaskSource.h"

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
		bool GetRef();
    private:
		int mRef;
        XCode mCode;
		lua_State * mLua;
        TaskSource<XCode> mTaskSource;
    };
}
#endif //GAMEKEEPER_LUATASKSOURCE_H

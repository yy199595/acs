//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef GAMEKEEPER_LUATASKSOURCE_H
#define GAMEKEEPER_LUATASKSOURCE_H
#include"Source/TaskSource.h"
#include"Http/HttpResponse.h"
namespace Sentry
{
    class LuaServiceTaskSource final
    {
    public:
        LuaServiceTaskSource();
        ~LuaServiceTaskSource() = default;
    public:
		static int SetRpc(lua_State * lua);
        static int SetHttp(lua_State* lua);
    public:
        XCode Await(Http::Response* message);
        XCode Await(std::shared_ptr<Message> message);
    private:		
        XCode mCode;
        Http::Response* mHttpData;
        TaskSource<void> mTaskSource;
        std::shared_ptr<Message> mRpcData;
    };
}

#endif //GAMEKEEPER_LUATASKSOURCE_H

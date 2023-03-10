//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef GAMEKEEPER_LUATASKSOURCE_H
#define GAMEKEEPER_LUATASKSOURCE_H
#include"Source/TaskSource.h"
#include"Http/HttpResponse.h"
#include"Lua/LuaInclude.h"
#include"google/protobuf/message.h"
using namespace google::protobuf;
namespace Sentry
{
    class LuaServiceTaskSource final
    {
    public:
        LuaServiceTaskSource(Http::Response* message);
        LuaServiceTaskSource(std::shared_ptr<Message> message);
    public:
		static int SetRpc(lua_State * lua);
        static int SetHttp(lua_State* lua);
    public:
        int Await();
    private:
        int mCode;
        Http::Response* mHttpData;
        TaskSource<void> mTaskSource;
        std::shared_ptr<Message> mRpcData;
    };
}

#endif //GAMEKEEPER_LUATASKSOURCE_H

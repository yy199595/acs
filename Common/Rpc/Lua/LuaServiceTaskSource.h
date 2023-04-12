//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef GAMEKEEPER_LUATASKSOURCE_H
#define GAMEKEEPER_LUATASKSOURCE_H
#include"Async/Source/TaskSource.h"
#include"Http/Common//HttpResponse.h"
#include"Lua/Engine/Define.h"
#include"google/protobuf/message.h"
using namespace google::protobuf;
namespace Tendo
{
    class LuaServiceTaskSource final
    {
    public:
        explicit LuaServiceTaskSource(Http::DataResponse* message);
        explicit LuaServiceTaskSource(std::shared_ptr<Message> message);
    public:
		static int SetRpc(lua_State * lua);
        static int SetHttp(lua_State* lua);
    public:
        int Await();
    private:
        int mCode;
        Http::DataResponse* mHttpData;
        TaskSource<void> mTaskSource;
        std::shared_ptr<Message> mRpcData;
    };
}

#endif //GAMEKEEPER_LUATASKSOURCE_H

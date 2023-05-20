//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef GAMEKEEPER_LUATASKSOURCE_H
#define GAMEKEEPER_LUATASKSOURCE_H
#include"Rpc/Client/Message.h"
#include"Async/Source/TaskSource.h"
#include"Http/Common//HttpResponse.h"
#include"Lua/Engine/Define.h"
namespace Tendo
{
    class LuaServiceTaskSource final
    {
    public:
		explicit LuaServiceTaskSource(Msg::Packet * packet);
		explicit LuaServiceTaskSource(Http::DataResponse* message);
	public:
		static int SetRpc(lua_State * lua);
        static int SetHttp(lua_State* lua);
	private:
		void WriteRpcResponse(lua_State * lua);
    public:
        int Await();
    private:
        int mCode;
		Msg::Packet * mRpcData;
		TaskSource<void> mTaskSource;
		Http::DataResponse* mHttpData;
    };
}

#endif //GAMEKEEPER_LUATASKSOURCE_H

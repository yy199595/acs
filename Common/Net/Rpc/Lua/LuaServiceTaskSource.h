//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef APP_LUASERVICETASKSOURCE_H
#define APP_LUASERVICETASKSOURCE_H
#include"Rpc/Client/Message.h"
#include"Lua/Engine/Define.h"
#include"Async/Source/TaskSource.h"
#include"Http/Common//HttpResponse.h"

namespace acs
{
	class LuaServiceTaskSource
    {
    public:
		explicit LuaServiceTaskSource(rpc::Message * packet);
		explicit LuaServiceTaskSource(http::Response* message);
		//~LuaServiceTaskSource() { printf("===================  %p \n", this); }
	public:
		static int SetRpc(lua_State * lua);
        static int SetHttp(lua_State* lua);
	private:
		void WriteRpcResponse(lua_State * lua);
    public:
        int Await();
    private:
        int mCode;
		rpc::Message * mRpcData;
		http::Response* mHttpData;
		TaskSource<void> mTaskSource;
	};
}

#endif //APP_LUASERVICETASKSOURCE_H

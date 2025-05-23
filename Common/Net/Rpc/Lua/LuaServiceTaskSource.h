//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef APP_LUASERVICETASKSOURCE_H
#define APP_LUASERVICETASKSOURCE_H
#include"Rpc/Common/Message.h"
#include"Lua/Engine/Define.h"
#include"Async/Source/TaskSource.h"
#include"Http/Common//HttpResponse.h"

namespace acs
{
	class LuaServiceTaskSource
#ifdef __SHARE_PTR_COUNTER__
	: public memory::Object<LuaServiceTaskSource>
#endif
    {
    public:
		explicit LuaServiceTaskSource(rpc::Message * packet);
		explicit LuaServiceTaskSource(http::Response* message);
		//~LuaServiceTaskSource() { printf("===================  %p \n", this); }
	public:
		static int SetRpc(lua_State * lua) noexcept;
        static int SetHttp(lua_State* lua) noexcept;
	private:
		void WriteRpcResponse(lua_State * lua) noexcept;
    public:
        int Await() noexcept;
    private:
        int mCode;
		rpc::Message * mRpcData;
		http::Response* mHttpData;
		TaskSource<void> mTaskSource;
	};
}

#endif //APP_LUASERVICETASKSOURCE_H

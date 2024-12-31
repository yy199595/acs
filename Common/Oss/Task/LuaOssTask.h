//
// Created by yy on 2024/10/9.
//

#ifndef APP_LUAOSSTASK_H
#define APP_LUAOSSTASK_H

#include"Http/Common/HttpResponse.h"
#include"Rpc/Async/RpcTaskSource.h"

namespace acs
{
	class LuaOssRequestTask : public IRpcTask<http::Response>
	{
	public:
		explicit LuaOssRequestTask(int id, lua_State * lua, std::string url);
		~LuaOssRequestTask() final;
	public:
		int Await();
		void OnResponse(std::unique_ptr<http::Response > response) final;
	private:
		int mRef;
		lua_State * mLua;
		std::string mUrl;
	};
}



#endif //APP_LUAOSSTASK_H

//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H
#include"DB/Mongo/MongoProto.h"
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	class MongoService final : public LocalRpcService, public ILuaRegister, public IStart
	{
	public:
		MongoService() = default;
	private:
		bool OnStart() final;
		bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
		bool OnStartService(ServiceMethodRegister &methodRegister) final;
	};
}


#endif //SERVER_MONGOSERVICE_H

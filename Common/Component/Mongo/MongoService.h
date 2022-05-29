//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H

#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	class MongoService final : public LocalRpcService, public IStart
	{
	public:
		MongoService() = default;
	protected:
		bool OnStart() final;
		bool LateAwake() final;
		bool OnStartService(ServiceMethodRegister &methodRegister) final;
	};
}


#endif //SERVER_MONGOSERVICE_H

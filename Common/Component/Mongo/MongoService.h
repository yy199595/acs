//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H

#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	class MongoService final : public LocalRpcService, public IComplete
	{
	public:
		MongoService() = default;
	protected:
		bool LateAwake() final;
        void OnAllServiceStart() final;
		bool OnStartService(ServiceMethodRegister &methodRegister) final;
	};
}


#endif //SERVER_MONGOSERVICE_H

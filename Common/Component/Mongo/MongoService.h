//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H
#include"DB/Mongo/MongoProto.h"
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	class MongoService final : public LocalRpcService
	{
	public:
		MongoService() = default;
	private:
		bool LateAwake() final;
		bool OnStartService(ServiceMethodRegister &methodRegister) final;
	 private:
		class MongoComponent * mMongoComponent;
	};
}


#endif //SERVER_MONGOSERVICE_H

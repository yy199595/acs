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
		MongoService();
	private:
		bool OnStartService(ServiceMethodRegister &methodRegister) final;
    private:
        XCode Insert(const s2s::Mongo::Insert & request);
        XCode Delete(const s2s::Mongo::Delete & request);
        XCode Update(const s2s::Mongo::Update & request);
        XCode Query(const s2s::Mongo::Query::Request & request, s2s::Mongo::Query::Response & response);
    private:
        std::string mBuffer;
		class MongoRpcComponent * mMongoComponent;
	};
}


#endif //SERVER_MONGOSERVICE_H

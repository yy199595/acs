

#ifndef APP_MONGODB_H
#define APP_MONGODB_H
#include"Message/s2s/db.pb.h"
#include"Mongo/Client/MongoProto.h"
#include "Mongo/Config/MongoConfig.h"
#include"Rpc/Service/RpcService.h"

namespace joke
{
    class MongoDB final : public RpcService
	{
	public:
		MongoDB();
	private:
		bool Awake() final;
        bool OnInit() final;
    private:
		int Save(const db::mongo::update& request);
		int Insert(const db::mongo::insert & request);
		int Delete(const db::mongo::remove & request);
		int Update(const db::mongo::update & request);
		int Updates(const db::mongo::updates & request);
		int SetIndex(const db::mongo::index & request);
		int Inc(const db::mongo::inc::request & request, db::mongo::inc::response & response);
		int Find(const db::mongo::find::request & request, db::mongo::find::response & response);
		int Count(const db::mongo::count::request & request, db::mongo::count::response & response);
		int Command(const db::mongo::command::request & request, db::mongo::command::response & response);
		int FindOne(const db::mongo::find_one::request & request, db::mongo::find_one::response & response);
		int FindPage(const db::mongo::find_page::request & request, db::mongo::find_page::response & response);
		int FindAndModify(const db::mongo::find_modify ::request & request, db::mongo::find_modify ::response & response);
	private:
		int UpdateData(const db::mongo::update & request, bool upsert);
	private:
        std::string mBuffer;
		class LoggerComponent * mLogger;
		class MongoDBComponent * mMongoComponent;
    };
}


#endif //APP_MONGODB_H

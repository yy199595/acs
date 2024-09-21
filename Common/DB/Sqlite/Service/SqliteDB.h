//
// Created by leyi on 2023/7/25.
//

#ifndef APP_SQLITEDB_H
#define APP_SQLITEDB_H
#include"Message/s2s/db.pb.h"
#include"Util/Sql/SqlHelper.h"
#include"Rpc/Service/RpcService.h"
namespace acs
{
	class ProtoComponent;
	class SqliteComponent;
	class SqliteDB : public RpcService
	{
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		int Add(const db::mysql::add & request);
		int Del(const db::mysql::remove & request);
		int Save(const db::mysql::save & request);
		int Update(const db::mysql::update & request);
		int Create(const db::mysql::create & request);
		int Find(const db::mysql::query & request, db::mysql::response & response);
		int FindOne(const db::mysql::query & request, db::mysql::response & response);
	private:
		SqlHelper mHelper;
		ProtoComponent * mProto;
		SqliteComponent * mComponent;
		std::unordered_map<std::string, int> mOpenDBs;
	};
}


#endif //APP_SQLITEDB_H

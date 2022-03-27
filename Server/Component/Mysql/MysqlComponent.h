#pragma once

#include"XCode/XCode.h"
#include"Component/Component.h"
#include"Util/Guid.h"
#include"rapidjson/document.h"
#include"DB/Mysql/MysqlDefine.h"


namespace Sentry
{
	class MysqlTaskAction;

	class MysqlTaskSource;

	class MysqlComponent : public Component
	{
	 public:
		MysqlComponent() = default;

		~MysqlComponent() final = default;

		bool LateAwake() final;

	 public:
		MysqlClient* ConnectMysql();
		MysqlClient* GetMysqlClient();
	 public:
		bool ToSqlCommand(const s2s::Mysql::Add& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Save& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Query& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Update& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Delete& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const std::string& table, const std::string& cmd, Message& message, std::string& sql);

	 protected:
		bool Awake() final;

	 private:
		bool WriterToStream(std::stringstream& stream, const rapidjson::Value& jsonValue);

	 private:

		bool StartConnect();
		bool InitMysqlTable();
		bool DropTable(const std::string& db);
		bool GetTableByProto(const Message& message, std::string& db);
	 private:
		std::string mMysqlIp;         //ip地址
		unsigned short mMysqlPort;     //端口号
		std::string mDataBaseUser;     //用户名
		std::string mDataBasePasswd; //密码
		MysqlClient* mMysqlSocket;
		std::stringstream mSqlCommandStream;
		std::stringstream mSqlCommandStream2;
		std::unordered_map<std::string, std::string> mSqlProtoMap;
		std::unordered_map<std::string, std::string> mSqlTableMap;
		std::unordered_map<std::thread::id, MysqlClient*> mMysqlSocketMap; //线程id和 socket
	 private:
		class TaskComponent* mCorComponent;
		class ThreadPoolComponent* mTaskManager;
	};
}
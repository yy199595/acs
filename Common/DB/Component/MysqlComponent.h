#pragma once

#include "XCode/XCode.h"
#include "Component/Component.h"
#include "Util/Guid.h"
#include "DB/MysqlClient/MysqlDefine.h"


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
		MysqlClient * ConnectMysql();
        MysqlClient * GetMysqlClient();
    public:
        bool GetTableName(const std::string &pb, std::string &table);

        bool GetProtoByTable(const std::string & tab, std::string & proto);
    public:
        bool GetAddSqlCommand(const Message &messageData, std::string &sqlCommand);

        bool GetSaveSqlCommand(const Message &messageData, std::string &sqlCommand);

        bool GetQuerySqlCommand(const Message &messageData, std::string &sqlCommand);

        bool GetDeleteSqlCommand(const Message &messageData, std::string &sqlCommand);

    protected:
        bool Awake() final;

    private:

		bool StartConnect();
        bool InitMysqlTable();
        bool DropTable(const std::string & db);
        bool GetTableByProto(const Message & message, std::string & db);
    private:
        std::string mMysqlIp;         //ip地址
        unsigned short mMysqlPort;     //端口号
        std::string mDataBaseUser;     //用户名
        std::string mDataBasePasswd; //密码
        MysqlClient *mMysqlSockt;
        std::stringstream mSqlCommandStream;
        std::stringstream mSqlCommandStream2;
        std::unordered_map<std::string, std::string> mSqlProtoMap;
        std::unordered_map<std::string, std::string> mSqlTableMap;
        std::unordered_map<std::thread::id, MysqlClient *> mMysqlSocketMap; //线程id和 socket
    private:
        class TaskComponent *mCorComponent;
        class ThreadPoolComponent *mTaskManager;
    };
}
#pragma once

#include "XCode/XCode.h"
#include "Component/Component.h"
#include "Util/Guid.h"
#include "MysqlClient/MysqlDefine.h"


namespace GameKeeper
{
    class MysqlTaskAction;

    class MysqlTaskSource;

    class MysqlComponent : public Component
    {
    public:
        MysqlComponent() = default;

        ~MysqlComponent() final = default;

    public:
        GKMysqlSocket *GetMysqlSocket();
		GKMysqlSocket * ConnectMysql();
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
        bool LateAwake() final;
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
        GKMysqlSocket *mMysqlSockt;
        std::stringstream mSqlCommandStream;
        std::stringstream mSqlCommandStream2;
        std::unordered_map<std::string, std::string> mSqlProtoMap;
        std::unordered_map<std::string, std::string> mSqlTableMap;
        std::unordered_map<std::thread::id, GKMysqlSocket *> mMysqlSocketMap; //线程id和 socket
    private:
        class TaskComponent *mCorComponent;
        class ThreadPoolComponent *mTaskManager;
    };
}
#pragma once

#include "XCode/XCode.h"
#include "Component/Component.h"
#include "Util/Guid.h"
#include "MysqlClient/MysqlDefine.h"

namespace GameKeeper
{
    class SqlTableConfig
    {
    public:
        SqlTableConfig(const std::string db, std::string  tab, std::string pb);

    public:
        void AddKey(const std::string& key);

        bool HasKey(const std::string &key) const;

    public:
        const std::string mDb;
        const std::string mTableName;
        const std::string mProtobufName;
        std::unordered_set<std::string> mKeys;
    };
}

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

        const SqlTableConfig *GetConfigByTab(const std::string &tab) const;
        const SqlTableConfig *GetCondifByProto(const std::string & proro) const;

        bool GetTableName(const std::string &pb, std::string &table);

    public:
        bool GetAddSqlCommand(const Message &messageData, std::string & db, std::string &sqlCommand);

        bool GetSaveSqlCommand(const Message &messageData,std::string & db, std::string &sqlCommand);

        bool GetQuerySqlCommand(const Message &messageData,std::string & db, std::string &sqlCommand);

        bool GetDeleteSqlCommand(const Message &messageData,std::string & db, std::string &sqlCommand);

    protected:
        bool Awake() final;
        bool LateAwake() final;
    private:

		bool StartConnect();
        bool InitMysqlTable();
        bool DropTable(const std::string & db, const std::string & name);

    private:
		std::string mSqlPath;
        std::string mMysqlIp;         //ip地址
        unsigned short mMysqlPort;     //端口号
        std::string mDataBaseUser;     //用户名
        std::string mDataBasePasswd; //密码
        GKMysqlSocket *mMysqlSockt;
        std::stringstream mSqlCommandStream;
        std::stringstream mSqlCommandStream2;
        std::unordered_map<std::string, SqlTableConfig *> mTablePbMap;
        std::unordered_map<std::string, SqlTableConfig *> mSqlConfigMap;   //sql表配置
        std::unordered_map<std::thread::id, GKMysqlSocket *> mMysqlSocketMap; //线程id和 socket
    private:
        class TaskComponent *mCorComponent;
        class ThreadPoolComponent *mTaskManager;
    };
}
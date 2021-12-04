#pragma once

#include <XCode/XCode.h>
#include <Component/Component.h>
#include <Util/NumberHelper.h>
#include <MysqlClient/MysqlDefine.h>

namespace GameKeeper
{
    class SqlTableConfig
    {
    public:
        SqlTableConfig(std::string  tab, std::string pb);

    public:
        void AddKey(const std::string& key);

        bool HasKey(const std::string &key) const;

    public:
        const std::string mTableName;
        std::vector<std::string> mKeys;
        const std::string mProtobufName;
    };
}

namespace GameKeeper
{
    class MysqlTaskAction;

    class MysqlTaskProxy;

    class MysqlComponent : public Component
    {
    public:
        MysqlComponent();

        ~MysqlComponent() final = default;

    public:
        GKMysqlSocket *GetMysqlSocket();
		GKMysqlSocket * ConnectMysql();
        const std::string &GetDataBaseName() { return this->mDataBaseName; }

    public:
        const SqlTableConfig *GetTableConfig(const std::string &tab) const;

        bool GetTableName(const std::string &pb, std::string &table);

        bool GetTableNameByProtocolName(const std::string &name, std::string &tableName);

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
        bool DropTable(const std::string & name);

    private:
		std::string mSqlPath;
        std::string mMysqlIp;         //ip地址
        unsigned short mMysqlPort;     //端口号
        std::string mDataBaseUser;     //用户名
        std::string mDataBasePasswd; //密码
        std::string mDataBaseName;     //数据库名字      
        GKMysqlSocket *mMysqlSockt{};
        std::stringstream mSqlCommandStream;
        std::stringstream mSqlCommandStream2;
        std::unordered_map<std::string, std::string> mTablePbMap;
        std::unordered_map<std::string, SqlTableConfig *> mSqlConfigMap;   //sql表配置
        std::unordered_map<std::thread::id, GKMysqlSocket *> mMysqlSocketMap; //线程id和 socket
    private:
        class TaskPoolComponent *mTaskManager{};
        class CoroutineComponent *mCorComponent{};
    };
}
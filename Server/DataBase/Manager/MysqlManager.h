#pragma once
#include <XCode/XCode.h>
#include <Manager/Manager.h>
#include <Util/NumberHelper.h>
#include <MysqlClient/MysqlDefine.h>
#include <QueryResult/InvokeResultData.h>

namespace SoEasy
{
	class SqlTableConfig
	{
	public:
		SqlTableConfig(const std::string tab, const std::string pb);

	public:
		void AddKey(const std::string key);
		bool HasKey(const std::string &key);

	public:
		const std::string mTableName;
		std::vector<std::string> mKeys;
		const std::string mProtobufName;
	};
}

namespace SoEasy
{
	class MysqlTaskAction;
	class MysqlTaskBase;
	class MysqlManager : public Manager
	{
	public:
		MysqlManager();
		~MysqlManager() {}

	public:
		SayNoMysqlSocket *GetMysqlSocket(long long threadId);
		const std::string &GetDataBaseName() { return this->mDataBaseName; }

	public:
		SqlTableConfig *GetTableConfig(const std::string &tab);
		bool GetTableName(const std::string &pb, std::string &table);
		shared_ptr<InvokeResultData> InvokeCommand(const std::string &sql);

	public:
		template <typename T>
		shared_ptr<T> CreateMysqlTask();
		XCode StartTask(shared_ptr<MysqlTaskBase> task, Message * messageData);
	protected:
		bool OnInit() final;
		void OnInitComplete() final;
		void PushClassToLua(lua_State *luaEnv) final; //自身方法导出到lua

	private:
		bool InitMysqlTable();
		bool StartConnectMysql();

	private:
		std::string mMysqlIp;		 //ip地址
		unsigned short mMysqlPort;	 //端口号
		std::string mDataBaseUser;	 //用户名
		std::string mDataBasePasswd; //密码
		std::string mDataBaseName;	 //数据库名字
		ThreadPool *mThreadPool;	 //线程池
		std::string mSqlTablePath;
		SayNoMysqlSocket *mMysqlSockt;
		std::unordered_map<std::string, std::string> mTablePbMap;
		std::unordered_map<std::string, SqlTableConfig *> mSqlConfigMap;   //sql表配置
		std::unordered_map<long long, SayNoMysqlSocket *> mMysqlSocketMap; //线程id和 socket
	private:
		class CoroutineManager *mCoroutineManager;
	};

	template <typename T>
	shared_ptr<T> MysqlManager::CreateMysqlTask()
	{
		long long id = NumberHelper::Create();
		return make_shared<T>(this, id, this->mDataBaseName);
	}
}
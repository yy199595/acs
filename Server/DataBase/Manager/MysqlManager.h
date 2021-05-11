#pragma once
#include<XCode/XCode.h>
#include<Manager/Manager.h>
#include<MysqlClient/MysqlDefine.h>
namespace DataBase
{
	class MysqlTaskAction;
	class MysqlManager : public SoEasy::Manager
	{
	public:
		MysqlManager();
		~MysqlManager() { }
	public:
		SayNoMysqlSocket * GetMysqlSocket(long long threadId);
	public:
		XCode QueryData(const std::string db, const std::string & sql);
		XCode QueryData(const std::string db, const std::string & sql, std::shared_ptr<MysqlQueryData> & queryData);
	protected:
		bool OnInit() override;
		void OnTaskFinish(long long id) final;
	private:
		bool StartConnectMysql();
	private:
		XCode QueryTable(long long id, shared_ptr<StringArray> requestData);
	private:
		std::string mMysqlIp;		//ip地址
		unsigned short mMysqlPort;	//端口号
		std::string mDataBaseUser;	//用户名
		std::string mDataBasePasswd; //密码
		ThreadPool * mThreadPool;		//线程池
		std::unordered_map<long long, SayNoMysqlSocket *> mMysqlSocketMap;	//线程id和 socket
	private:
		class CoroutineManager * mCoroutineManager;
		std::unordered_map<long long, shared_ptr<MysqlTaskAction>> mTaskActionMap;
	};
}
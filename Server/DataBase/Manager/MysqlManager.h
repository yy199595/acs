#pragma once
#include<XCode/XCode.h>
#include<Manager/Manager.h>
#include<MysqlClient/MysqlDefine.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	class MysqlTaskAction;
	class MysqlManager : public Manager
	{
	public:
		MysqlManager();
		~MysqlManager() { }
	public:
		SayNoMysqlSocket * GetMysqlSocket(long long threadId);
	public:
		shared_ptr<InvokeResultData> InvokeCommand(const std::string db, const std::string & sql);
	protected:
		bool OnInit() final;
		void OnInitComplete() final;
		void PushClassToLua(lua_State * luaEnv) final;		//自身方法导出到lua

	private:
		bool StartConnectMysql();
	private:
		std::string mMysqlIp;		//ip地址
		unsigned short mMysqlPort;	//端口号
		std::string mDataBaseUser;	//用户名
		std::string mDataBasePasswd; //密码
		ThreadPool * mThreadPool;		//线程池
		std::unordered_map<long long, SayNoMysqlSocket *> mMysqlSocketMap;	//线程id和 socket
	private:
		class CoroutineManager * mCoroutineManager;
	};
}
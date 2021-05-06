#pragma once
#include<CommonManager/Manager.h>
#include<MysqlClient/MysqlDefine.h>

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
		XMysqlErrorCode QueryData(const char * db, const std::string & sql);
		XMysqlErrorCode QueryData(const char * db, const std::string & sql, std::shared_ptr<MysqlQueryData> & queryData);
	protected:
		bool OnInit() override;
		void OnTaskFinish(long long id) override;
		void OnRegisterSuccessful(shared_ptr<TcpClientSession> tcpSession) override;

	private:
		bool StartConnectMysql();
		bool ConnectRedis();
	private:
		SayNoRedisConfig mRedisConfig;
		SayNoMySqlConfig mMysqlConfig;
		std::unordered_map<long long, SayNoMysqlSocket *> mMysqlSocketMap;	//Ïß³ÌidºÍ socket
	private:
		std::unordered_map<long long, shared_ptr<MysqlTaskAction>> mTaskActionMap;
	};
	TYPE_REFLECTION(MysqlManager);
}
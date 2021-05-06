#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#endif
#include<vector>
#include<sstream>
#include"mysql.h"
#include<memory>
#include<unordered_map>
#include<Thread/ThreadTaskAction.h>
using namespace std;
namespace SoEasy
{
	class MysqlManager;
	typedef  MYSQL_RES MysqlQueryResult;
	typedef MYSQL SayNoMysqlSocket;
}

namespace SoEasy
{
	enum XMysqlErrorCode
	{
		MysqlSuccessful,		//成功
		MysqlFailure,			//失败
		MysqlNotInCoroutine,	//不在协程中
		MysqlStartTaskFail,		//启动失败
		MysqlSocketIsNull,		//socket空
		MysqlSelectDbFailure,	//选择db失败
		MysqlInvokeFailure,		//执行sql语句失败
	};
}

namespace SoEasy
{
	class MysqlQueryLine
	{
	public:
		bool AddMember(const std::string & key, const std::string & value);
	public:
		const int GetInt32Field(const char * key);
		const float GetFloat32Field(const char * key);
		const double GetFloat64Field(const char * key);
		const long long GetInt64Field(const char * key);
		const std::string & GetStringField(const char * key);
	private:
		std::unordered_map<std::string, std::string> mContent;
	};
}

namespace SoEasy
{
	class MysqlQueryData
	{
	public:
		
		MysqlQueryData() { }
		~MysqlQueryData() { }
	public:
		friend class MysqlTaskAction;
		size_t GetRowCount() { return mFiledMap.size(); }	//获取有多少个字段
		size_t GetColumnCount() { return mContentVector.size(); }	//获取有多少行
		XMysqlErrorCode GetErrorCode() { return this->mErrorCode; }
		std::shared_ptr<MysqlQueryLine> GetLineData(size_t col = 0);	//获取一行数据
		const std::string & GetErrorMessage() { return this->mErrorMessage; }	//获取错误消息
	public:
		bool AddFieldName(const char * name, size_t len, int pos);
		void AddFieldContent(int row, const char * data, size_t len);
		void SetErrorCode(XMysqlErrorCode code) { this->mErrorCode = code; }
		void SetErrorMessage(const char * msg) { this->mErrorMessage = msg; }
	private:
		std::string mErrorMessage;
		XMysqlErrorCode mErrorCode;
		std::unordered_map<std::string, int> mFiledMap;
		std::vector<std::vector<std::string>> mContentVector;
	};
}

namespace SoEasy
{
	struct SayNoMySqlConfig
	{
		SayNoMySqlConfig() : mPort(0) { }
	public:
		std::string mIp;
		unsigned short mPort;
		std::string mUserName;
		std::string mPassCode;
	public:
		const std::string ToString()
		{
			std::stringstream buffer;
			buffer << "ip:" << mIp << ",";
			buffer << "port:" << mPort << ",";
			buffer << "username:" << mUserName << ",";
			return buffer.str();
		}
	};

	struct SayNoRedisConfig
	{
	public:
		SayNoRedisConfig() : mPort(0) {}
	public:
		std::string mIp;
		unsigned short mPort;
	};
}

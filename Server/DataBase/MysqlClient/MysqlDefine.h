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
#include<XCode/XCode.h>
#include<Thread/ThreadTaskAction.h>
using namespace std;
using namespace SoEasy;
namespace SoEasy
{
	class MysqlManager;
	typedef  MYSQL_RES MysqlQueryResult;
	typedef MYSQL SayNoMysqlSocket;
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
		XCode GetErrorCode() { return this->mErrorCode; }
		std::shared_ptr<MysqlQueryLine> GetLineData(size_t col = 0);	//获取一行数据
		const std::string & GetErrorMessage() { return this->mErrorMessage; }	//获取错误消息
	public:
		bool AddFieldName(const char * name, size_t len, int pos);
		void AddFieldContent(int row, const char * data, size_t len);
		void SetErrorCode(XCode code) { this->mErrorCode = code; }
		void SetErrorMessage(const char * msg) { this->mErrorMessage = msg; }
	public:
		void DebugPrint();
	private:
		XCode mErrorCode;
		std::string mErrorMessage;
		std::unordered_map<std::string, int> mFiledMap;
		std::vector<std::vector<std::string>> mContentVector;
	};
}

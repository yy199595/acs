#pragma once
#include"Sqlite/sqlite3.h"
#include<unordered_map>
#include"Proto/Include/Message.h"
#include"Util/Tools/NumberBuilder.h"
#include"Entity/Component/Component.h"
namespace acs
{
	class SqlHelper;
	class SqliteComponent final : public Component, public IDestroy
	{
	public:
		SqliteComponent() = default;
	public:
		void Close(const std::string & db);
		int Exec(const std::string & tab, const char * sql);
		int Exec(const std::string & tab, const char * sql, std::string & err);
		int Query(const std::string & tab, const char * sql, std::vector<std::string> & result);
		int MakeTable(const std::string & tab, const pb::Message & message, const std::vector<std::string> & keys);
	private:
		bool Awake() final;
		void OnDestroy() final;
		sqlite3 * GetClient(const std::string & tab);
	private:
		std::string mName;
		std::string mSaveDir;
		help::NumberBuilder<int, 10> mNumbers;
		std::unordered_map<std::string, sqlite3 *> mDatabases;
	};
}
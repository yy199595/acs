#pragma once
#include"Sqlite/sqlite3.h"
#include<unordered_map>
#include"Proto/Include/Message.h"
#include"Util/Guid/NumberBuilder.h"
#include"Entity/Component/Component.h"
namespace joke
{
	class SqlHelper;
	class SqliteComponent final : public Component, public ILuaRegister, public IDestroy
	{
	public:
		SqliteComponent() = default;
	public:
		void Close(const std::string & db);
		int Exec(const std::string & tab, const char * sql);
		int Exec(const std::string & tab, const char * sql, std::string & err);
		static bool Splite(const std::string & table, std::string & db, std::string & tab);
		int Query(const std::string & tab, const char * sql, std::vector<std::string> & result);
		int MakeTable(const std::string & tab, const pb::Message & message, const std::vector<std::string> & keys);
	private:
		bool Awake() final;
		void OnDestroy() final;
		sqlite3 * GetClient(const std::string & tab);
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
	private:
		std::string mName;
		std::string mSaveDir;
		help::NumberBuilder<int, 10> mNumbers;
		std::unordered_map<std::string, sqlite3 *> mDatabases;
	};
}
#pragma once
#include"Sqlite/sqlite3.h"
#include<unordered_map>
#include"Proto/Include/Message.h"
#include"Util/Tools/NumberBuilder.h"
#include"Entity/Component/Component.h"
#include "Yyjson/Object/JsonObject.h"
namespace sqlite
{
	struct Config : public json::Object<Config>
	{
	public:
		std::string db;
		std::string dir;
		std::string script;
	};
}

namespace acs
{
	class SqlHelper;
	class SqliteComponent final : public Component, public IDestroy, public IHotfix
	{
	public:
		SqliteComponent();
	public:
		bool Exec(const char * sql);
		bool Exec(const char * sql, std::string & err);
		bool Query(const char * sql, std::vector<std::string> & result);
		int MakeTable(const std::string & tab, const pb::Message & message, const std::vector<std::string> & keys);
	public:
		bool Del(const std::string & key);
		bool Get(const std::string & key, std::string & value);
		bool SetTimeout(const std::string & key, int timeout);
		bool Set(const std::string & key, const std::string & value);
	private:
		bool Awake() final;
		bool OnHotFix() final;
		bool LateAwake() final;
		void OnDestroy() final;
		bool InvokeSqlFromPath(const std::string & path);
	private:
		std::string mName;
		sqlite3 * mDatabase;
		sqlite::Config mConfig;
	};
}
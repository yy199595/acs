#include"Sqlite/sqlite3.h"
#include<unordered_map>
#include"Proto/Include/Message.h"
#include"Util/Guid/NumberBuilder.h"
#include"Entity/Component/Component.h"
namespace Tendo
{
	class SqlHelper;
	class SqliteComponent final : public Component, public ILuaRegister, public IDestroy
	{
	public:
		SqliteComponent() = default;
	public:
		void Close(int id);
		int Open(const std::string & name);
		bool Exec(int id, const char * sql);
		bool Query(int id, const char * sql, std::vector<std::string> & result);
		bool MakeTable(int id, const std::string & key, const pb::Message & message);
	private:
		bool Awake() final;
		void OnDestroy() final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister, std::string &name) final;
	private:
		std::string mName;
		std::string mPath;
		Util::NumberBuilder<int, 10> mNumbers;
		std::unordered_map<int, sqlite3 *> mDatabases;
	};
}
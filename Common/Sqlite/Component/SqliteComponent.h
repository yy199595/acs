#include"sqlite3.h"
#include"Component/Component.h"

namespace Sentry
{
	class SqliteComponent : public Component
	{
	public:
		SqliteComponent() = default;
		~SqliteComponent() = default;
	public:
		bool OpenDataBase(const std::string& name);
		bool ExecSql(const std::string& sql);
	public:
		bool LateAwake() final;
	private:
		sqlite3 * mDb;
		std::string mName;
		std::string mPath;
	};
}
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
		bool Exec(const std::string& sql);
		bool OpenDataBase(const std::string& name);
	public:
		bool LateAwake() final;
	private:
		sqlite3 * mDb;
		std::string mName;
		std::string mPath;
	};
}
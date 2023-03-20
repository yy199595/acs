#include"sqlite3.h"
#include<unordered_map>
#include"Component/Component.h"

namespace Sentry
{
	class SqliteComponent : public Component
	{
	public:
		SqliteComponent() = default;
		~SqliteComponent() = default;
	public:
		bool Exec(const std::string & db, const std::string& sql);
	public:
		bool LateAwake() final;
	private:
		std::string mName;
		std::string mPath;
		std::unordered_map<std::string, sqlite3 *> mDbs;
	};
}
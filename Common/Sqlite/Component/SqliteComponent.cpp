#include"SqliteComponent.h"
#include"Config/ServerConfig.h"
namespace Sentry
{
	static int SqlCallback(void *, int, char **, char **)
	{
		return 0;
	}
	bool SqliteComponent::LateAwake()
	{
		this->mDb = nullptr;
		if (!ServerConfig::Inst()->GetPath("sqlite", this->mPath))
		{
			return false;
		}
		const std::string& name = ServerConfig::Inst()->Name();
		this->mPath = fmt::format("{0}/{1}.db", this->mPath, name);
		return sqlite3_open(this->mPath.c_str(), &mDb) == 0;		
	}
	bool SqliteComponent::ExecSql(const std::string& sql)
	{
		char* errMessage = 0;
		int code = sqlite3_exec(this->mDb, sql.c_str(), SqlCallback, 0, &errMessage);
		if (code != SQLITE_OK)
		{
			LOG_ERROR(errMessage);
			sqlite3_free(errMessage);
			return false;
		}
		return true;
	}
}
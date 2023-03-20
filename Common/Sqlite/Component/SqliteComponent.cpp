#include"SqliteComponent.h"
#include"Config/ServerConfig.h"
#include"File/DirectoryHelper.h"
namespace Sentry
{
	static int SqlCallback(void *, int, char **, char **)
	{
		return 0;
	}
	bool SqliteComponent::LateAwake()
	{
		if (!ServerConfig::Inst()->GetPath("sqlite", this->mPath))
		{
			return false;
		}
		std::vector<std::string> paths;
		Helper::Directory::GetFilePaths(this->mPath, "*.db",paths);
		for(const std::string & path : paths)
		{
			std::string name;
			if(!Helper::Directory::GetFileName(path, name))
			{
				return false;
			}
			sqlite3 * db = nullptr;
			if(sqlite3_open(path.c_str(), &db) != 0)
			{
				return false;
			}
			this->mDbs.emplace(name, db);
		}
		return true;
	}

	bool SqliteComponent::Exec(const std::string & db, const std::string& sql)
	{
		auto iter = this->mDbs.find(db);
		if(iter == this->mDbs.end())
		{
			return false;
		}
		char* errMessage = 0;
		int code = sqlite3_exec(iter->second, sql.c_str(), SqlCallback, 0, &errMessage);
		if (code != SQLITE_OK)
		{
			LOG_ERROR(errMessage);
			sqlite3_free(errMessage);
			return false;
		}
		return true;
	}
}
#include"SqliteComponent.h"
#include"Config/ServerConfig.h"
namespace Sentry
{
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
}
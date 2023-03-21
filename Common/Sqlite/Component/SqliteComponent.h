#include"sqlite3.h"
#include<unordered_map>
#include"Guid/NumberBuilder.h"
#include"Component/Component.h"
#include"google/protobuf/message.h"
namespace Sentry
{
	class SqliteComponent : public Component
	{
	public:
		SqliteComponent() = default;
		~SqliteComponent() = default;
	public:
		void Close(int id);
		int Open(const std::string & name);
		bool Exec(int id, const std::string& sql);
		bool Query(int id, const std::string & sql, std::vector<std::string> & result);
		bool MakeTable(int id, const std::string & key, const google::protobuf::Message & message);
	 public:
		bool LateAwake() final;
		void OnDestroy() final;
	private:
		std::string mName;
		std::string mPath;
		Util::NumberBuilder<int, 10> mNumbers;
		std::unordered_map<int, sqlite3 *> mDatabases;
	};
}
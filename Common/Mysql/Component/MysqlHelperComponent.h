
#ifdef __ENABLE_MYSQL__


#include"XCode/XCode.h"
#include"Message/s2s/db.pb.h"
#include"Entity/Component/Component.h"
#include"google/protobuf/util/json_util.h"
using namespace google::protobuf;
namespace Tendo
{
    class MysqlHelperComponent final : public Component
	{
	 public:
		MysqlHelperComponent() = default;
     public:
		int Add(const Message & data, int flag);

		int Save(const Message & data, int flag);

		int QueryOnce(const std::string & json, std::shared_ptr<Message> response);

        int Delete(const std::string & table, const std::string& deleteJson, int flag);

        int Update(const std::string & table, const std::string& updateJson, const std::string& whereJson, int flag);

    private:
		bool Awake() final;
	private:
		std::string mServer;
	};
}

#endif
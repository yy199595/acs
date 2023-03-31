
#include"XCode/XCode.h"
#include"Message/db.pb.h"
#include"Core/Component/Component.h"
#include"google/protobuf/util/json_util.h"
using namespace google::protobuf;
namespace Sentry
{
    class MysqlHelperComponent final : public Component
	{
	 public:
		MysqlHelperComponent() = default;
     public:
		int Add(const Message & data, int flag);

		int Save(const Message & data, int flag);

		int QueryOnce(const std::string & json, std::shared_ptr<Message> response);

        template<typename T>
		std::vector<std::shared_ptr<T>> QueryAll(const std::string& queryJson);

		template<typename T>
		int Delete(const std::string& deleteJson, int flag);

        int Delete(const std::string & table, const std::string& deleteJson, int flag);

        template<typename T>
		int Update(const std::string& updateJson, const std::string& whereJson, int flag);

        int Update(const std::string & table, const std::string& updateJson, const std::string& whereJson, int flag);

    private:
		bool LateAwake() final;
		int Call(const std::string& func, const Message& data, std::shared_ptr<db::mysql::response> response = nullptr);
	private:
		std::string mServerName;
		class RpcService* mMysqlDB;
		class NodeMgrComponent * mLocationComponent;
	};

	template<typename T>
	std::vector<std::shared_ptr<T>>MysqlHelperComponent::QueryAll(const std::string& queryJson)
	{
		std::shared_ptr<T> queryData(new T());

		db::mysql::query request;
		request.set_where_json(queryJson);
		request.set_table(queryData->GetTypeName());

		std::shared_ptr<db::mysql::response>
			response = std::make_shared<db::mysql::response>();
		if(this->Call("Query", request, response) != XCode::Successful)
		{
			return std::vector<std::shared_ptr<T>>();
		}
		std::vector<std::shared_ptr<T>> respArray;
		for (int index = 0; index < response->jsons_size(); index++)
		{
			std::shared_ptr<T> jsonData(new T());
            const std::string & json = response->jsons(index);
			if(util::JsonStringToMessage(json, jsonData.get()).ok())
			{
				respArray.emplace_back(jsonData);
			}
		}
		return respArray;
	}

	template<typename T>
	int MysqlHelperComponent::Delete(const std::string& deleteJson, int flag)
	{
        T data;
        return this->Delete(data.GetTypeName(), deleteJson, flag);
	}

	template<typename T>
	int MysqlHelperComponent::Update(const std::string& updateJson, const std::string& whereJson, int flag)
	{
        T data;
        return this->Update(data.GetTypeName(), whereJson, updateJson, flag);
	}
}
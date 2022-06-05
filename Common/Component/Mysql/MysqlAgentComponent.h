#include"Component/Component.h"
#include"google/protobuf/util/json_util.h"
#include"DB/Mysql/MysqlRpcTaskSource.h"
namespace Sentry
{
	class MysqlRpcTaskSource;
	class MysqlAgentComponent : public Component
	{
	 public:
		MysqlAgentComponent() = default;
		~MysqlAgentComponent() final = default;
		MysqlAgentComponent(const MysqlAgentComponent &) = delete;

	public:
		XCode Add(const Message & data, long long flag = 0);

		XCode Save(const Message & data, long long flag = 0);

		XCode QueryOnce(const std::string & json, std::shared_ptr<Message> response, long long flag = 0);

		template<typename T>
		std::vector<std::shared_ptr<T>> QueryAll(const std::string& queryJson, long long flag = 0);

		template<typename T>
		XCode Delete(const std::string& deleteJson, long long flag = 0);

		template<typename T>
		XCode Update(const std::string& updateJson, const std::string& whereJson, long long flag = 0);

	 private:
		bool LateAwake() final;
		XCode Call(const std::string& func, const Message& data, std::shared_ptr<s2s::Mysql::Response> response = nullptr);
	private:
		class MysqlService * mMysqlService;
	};

	template<typename T>
	std::vector<std::shared_ptr<T>>MysqlAgentComponent::QueryAll(const std::string& queryJson, long long flag)
	{
		std::shared_ptr<T> queryData(new T());

		s2s::Mysql::Query request;
		request.set_flag(flag);
		request.set_where_json(queryJson);
		request.set_table(queryData->GetTypeName());

		std::shared_ptr<s2s::Mysql::Response>
			response = std::make_shared<s2s::Mysql::Response>();
		if(this->Call("Query", request, response) != XCode::Successful)
		{
			return std::vector<std::shared_ptr<T>>();
		}
		std::vector<std::shared_ptr<T>> respArray;
		for (int index = 0; index < response->json_array_size(); index++)
		{
			const std::string& json = response->json_array(index);
			std::shared_ptr<T> jsonData(new T());
			if (util::JsonStringToMessage(json, jsonData.get()).ok())
			{
				respArray.emplace_back(jsonData);
			}
		}
		return respArray;
	}

	template<typename T>
	XCode MysqlAgentComponent::Delete(const std::string& deleteJson, long long flag)
	{
		s2s::Mysql::Delete request;
		request.set_flag(flag);
		std::shared_ptr<T> data(new T());
		request.set_table(data->GetTypeName());
		request.set_where_json(deleteJson);
		return this->Call("Delete", request);
	}

	template<typename T>
	XCode MysqlAgentComponent::Update(const std::string& updateJson, const std::string& whereJson, long long flag)
	{
		s2s::Mysql::Update request;
		std::shared_ptr<T> data(new T());
		request.set_table(data->GetTypeName());

		request.set_flag(flag);
		request.set_where_json(whereJson);
		request.set_update_json(updateJson);
		return this->Call("Update", request);
	}
}
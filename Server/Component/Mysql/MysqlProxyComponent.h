#include"Component/Component.h"
#include"Protocol/s2s.pb.h"
#include"google/protobuf/util/json_util.h"
#include"DB/Mysql/MysqlRpcTaskSource.h"
namespace Sentry
{
	class MysqlRpcTaskSource;
	class MysqlProxyComponent : public Component
	{
	 public:
		MysqlProxyComponent() = default;

		~MysqlProxyComponent() final = default;

	 public:
		XCode Add(const Message & data);

		XCode Save(const Message & data);

		XCode QueryOnce(const std::string & json, std::shared_ptr<Message> response);

		template<typename T>
		std::vector<std::shared_ptr<T>> QueryAll(const std::string& queryJson);

		template<typename T>
		XCode Delete(const std::string& deleteJson);

		template<typename T>
		XCode Update(const std::string& updateJson, const std::string& whereJson);

		std::shared_ptr<s2s::Mysql::Response> Invoke(const std::string& sql);

		template<typename T>
		std::vector<std::shared_ptr<T>> Sort(const std::string& field, int count, bool reverse = false);

	 private:
		XCode Call(const std::string& func, const Message& data, std::shared_ptr<s2s::Mysql::Response> response);
	};

	template<typename T>
	std::vector<std::shared_ptr<T>>MysqlProxyComponent::QueryAll(const std::string& queryJson)
	{
		std::shared_ptr<T> queryData(new T());

		s2s::Mysql::Query request;
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
	XCode MysqlProxyComponent::Delete(const std::string& deleteJson)
	{
		std::shared_ptr<T> data(new T());
		s2s::Mysql::Delete request;
		request.set_table(data->GetTypeName());
		request.set_where_json(deleteJson);

		std::shared_ptr<s2s::Mysql::Response>
			response = std::make_shared<s2s::Mysql::Response>();
		return this->Call("Delete", request, response);
	}

	template<typename T>
	std::vector<std::shared_ptr<T>>
	MysqlProxyComponent::Sort(const std::string& field, int count, bool reverse)
	{
		std::shared_ptr<T> data(new T());
		const char* type = !reverse ? "ASC" : "DESC";
		auto response = this->Invoke(fmt::format(
			"select * from {0} ORDER BY {1} {2} LIMIT {3}", data->GetTypeName(), field, type, count));
		std::vector<std::shared_ptr<T>> respArray;
		if (response != nullptr && response->json_array_size() > 0)
		{
			for (int index = 0; index < response->json_array_size(); index++)
			{
				std::shared_ptr<T> jsonData(new T());
				const std::string& json = response->json_array(index);
				if (util::JsonStringToMessage(json, jsonData.get()))
				{
					respArray.template emplace_back(jsonData);
				}
			}
		}
		return respArray;
	}

	template<typename T>
	XCode MysqlProxyComponent::Update(const std::string& updateJson, const std::string& whereJson)
	{
		s2s::Mysql::Update request;
		std::shared_ptr<T> data(new T());
		request.set_table(data->GetTypeName());
		request.set_where_json(whereJson);
		request.set_update_json(updateJson);

		std::shared_ptr<s2s::Mysql::Response>
			response = std::make_shared<s2s::Mysql::Response>();
		return this->Call("Update", request, response);
	}
}
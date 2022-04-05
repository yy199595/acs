#include"Component/Component.h"
#include "Pool/ObjectPool.h"
#include"Protocol/s2s.pb.h"
#include"google/protobuf/util/json_util.h"
#include"DB/Mysql/MysqlRpcTaskSource.h"
namespace Sentry
{
	class ServiceProxy;
	class MysqlRpcTaskSource;
	class MysqlProxyComponent : public Component
	{
	 public:
		MysqlProxyComponent() = default;

		~MysqlProxyComponent() final = default;

	 protected:
		bool Awake() final;

		bool LateAwake() final;
	 private:
		std::shared_ptr<com::Rpc_Request> NewMessage(const std::string& name);
	 public:
		template<typename T>
		XCode Add(const T& data);

		template<typename T>
		XCode Save(const T& data);

		template<typename T>
		std::shared_ptr<T> QueryOnce(const std::string& queryJson);

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
		std::shared_ptr<MysqlRpcTaskSource> Call(const std::string& func, const Message& data);
	 private:
		class RpcComponent* mRpcComponent;
		class TaskComponent* mCorComponent;
		class ServiceMgrComponent* mServiceComponent;
	};

	template<typename T>
	XCode
	MysqlProxyComponent::Add(const T& data)
	{
		s2s::Mysql::Add request;
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		auto taskSource = this->Call("Add", request);
		return taskSource != nullptr ? taskSource->GetCode() : XCode::Failure;
	}

	template<typename T>
	XCode
	MysqlProxyComponent::Save(const T& data)
	{
		s2s::Mysql::Save request;
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		auto taskSource = this->Call("Save", request);
		return taskSource == nullptr ? XCode::Failure : taskSource->GetCode();
	}

	template<typename T>
	std::shared_ptr<T> MysqlProxyComponent::QueryOnce(const std::string& queryJson)
	{
		std::shared_ptr<T> queryData(new T());

		s2s::Mysql::Query request;
		request.set_where_json(queryJson);
		request.set_table(queryData->GetTypeName());
		auto taskSource = this->Call("Query", request);
		if (taskSource == nullptr)
		{
			return nullptr;
		}
		auto response = taskSource->GetResponse();
		if (response != nullptr && response->json_array_size() > 0)
		{
			const std::string& json = response->json_array(0);
			util::Status status = util::JsonStringToMessage(json, queryData.get());
			return status.ok() ? queryData : nullptr;
		}
		return nullptr;
	}

	template<typename T>
	std::vector<std::shared_ptr<T>>
	MysqlProxyComponent::QueryAll(const std::string& queryJson)
	{
		std::shared_ptr<T> queryData(new T());

		s2s::Mysql::Query request;
		request.set_where_json(queryJson);
		request.set_table(queryData->GetTypeName());
		auto taskSource = this->Call("Query", request);
		if (taskSource == nullptr)
		{
			return std::vector<std::shared_ptr<T>>();;
		}
		std::vector<std::shared_ptr<T>> respArray;
		auto response = taskSource->GetResponse();
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
		request.set_where_json(deleteJson);
		request.set_table(data->GetTypeName());
		auto taskSource = this->Call("Delete", request);
		return taskSource == nullptr ? XCode::Failure : taskSource->GetCode();
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
		auto response = this->Call("Update", request);
		return response == nullptr ? XCode::Failure : response->GetCode();
	}
}
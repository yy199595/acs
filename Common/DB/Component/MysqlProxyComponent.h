#include"Component/Component.h"
#include "Pool/ObjectPool.h"
#include"Protocol/s2s.pb.h"
#include"Util/JsonHelper.h"
#include"DB/MysqlClient/MysqlRpcTaskSource.h"
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
		bool
		Awake() final;

		bool
		LateAwake() final;

		void
		OnComplete() final;
	 private:
		void
		AddUserData();
		std::shared_ptr<com::Rpc_Request>
		NewMessage(const std::string& name);
	 public:
		template<typename T>
		XCode
		Add(const T& data);

		template<typename T>
		XCode
		Save(const T& data);

		template<typename T>
		std::shared_ptr<T>
		QueryOnce(RapidJsonWriter& queryJson);

		template<typename T>
		std::vector<std::shared_ptr<T>>
		QueryAll(RapidJsonWriter& queryJson);

		template<typename T>
		XCode Delete(RapidJsonWriter& deleteJson);

		template<typename T>
		XCode Update(RapidJsonWriter& updateJson, RapidJsonWriter& whereJson);

		std::shared_ptr<s2s::Mysql::Response>
		Invoke(const std::string& sql);

		template<typename T>
		std::vector<std::shared_ptr<T>> Sort(const std::string& field, int count, bool reverse = false);

	 private:
		std::shared_ptr<MysqlRpcTaskSource>
		Call(const std::string& func, const Message& data);
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
	std::shared_ptr<T>
	MysqlProxyComponent::QueryOnce(RapidJsonWriter& queryJson)
	{
		std::string json;
		if (!queryJson.WriterToStream(json))
		{
			return nullptr;
		}
		std::shared_ptr<T> queryData(new T());

		s2s::Mysql::Query request;
		request.set_where_json(json);
		request.set_table(queryData->GetTypeName());
		auto taskSource = this->Call("Query", request);
		if (taskSource == nullptr)
		{
			return nullptr;
		}
		auto response = taskSource->GetResponse();
		if (response != nullptr && response->json_array_size() > 0)
		{
			json = response->json_array(0);
			util::Status status = util::JsonStringToMessage(json, queryData.get());
			return status.ok() ? queryData : nullptr;
		}
		return nullptr;
	}

	template<typename T>
	std::vector<std::shared_ptr<T>>
	MysqlProxyComponent::QueryAll(RapidJsonWriter& queryJson)
	{
		std::string json;
		if (!queryJson.WriterToStream(json))
		{
			return std::vector<std::shared_ptr<T>>();
		}
		std::shared_ptr<T> queryData(new T());

		s2s::Mysql::Query request;
		request.set_where_json(json);
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
			json = response->json_array(index);
			std::shared_ptr<T> jsonData(new T());
			if (util::JsonStringToMessage(json, jsonData.get()).ok())
			{
				respArray.emplace_back(jsonData);
			}
		}
		return respArray;
	}

	template<typename T>
	XCode
	MysqlProxyComponent::Delete(RapidJsonWriter& deleteJson)
	{
		std::string json;
		std::shared_ptr<T> data(new T());
		if (!deleteJson.WriterToStream(json))
		{
			return XCode::CallArgsError;
		}

		s2s::Mysql::Delete request;
		request.set_where_json(json);
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
	XCode MysqlProxyComponent::Update(RapidJsonWriter& updateJson, RapidJsonWriter& whereJson)
	{
		std::string whereJsonStr;
		std::string updateJsonStr;
		if (!whereJson.WriterToStream(whereJsonStr) || !updateJson.WriterToStream(updateJsonStr))
		{
			return XCode::CallArgsError;
		}
		s2s::Mysql::Update request;
		std::shared_ptr<T> data(new T());
		request.set_table(data->GetTypeName());
		request.set_where_json(whereJsonStr);
		request.set_update_json(updateJsonStr);
		auto response = this->Call("Update", request);
		return response == nullptr ? XCode::Failure : response->GetCode();
	}
}
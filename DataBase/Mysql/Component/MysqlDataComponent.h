#include"Component/Component.h"
#include"google/protobuf/util/json_util.h"
namespace Sentry
{
    class MysqlDataComponent final : public Component
	{
	 public:
		MysqlDataComponent() = default;
		~MysqlDataComponent() final = default;
		MysqlDataComponent(const MysqlDataComponent &) = delete;

	public:
		XCode Add(const Message & data, long long flag = 0);

		XCode Save(const Message & data, long long flag = 0);

		XCode QueryOnce(const std::string & json, std::shared_ptr<Message> response, long long flag = 0);

        template<typename T>
		std::vector<std::shared_ptr<T>> QueryAll(const std::string& queryJson, long long flag = 0);

		template<typename T>
		XCode Delete(const std::string& deleteJson, long long flag = 0);

        XCode Delete(const std::string & table, const std::string& deleteJson, long long flag = 0);

        template<typename T>
		XCode Update(const std::string& updateJson, const std::string& whereJson, long long flag = 0);

        XCode Update(const std::string & table, const std::string& updateJson, const std::string& whereJson, long long flag = 0);

    private:
		bool LateAwake() final;
		XCode Call(const std::string& func, const Message& data, std::shared_ptr<s2s::mysql::response> response = nullptr);
	private:
		class MysqlService * mMysqlService;
	};

	template<typename T>
	std::vector<std::shared_ptr<T>>MysqlDataComponent::QueryAll(const std::string& queryJson, long long flag)
	{
		std::shared_ptr<T> queryData(new T());

		s2s::mysql::query request;
		request.set_flag(flag);
		request.set_where_json(queryJson);
		request.set_table(queryData->GetTypeName());

		std::shared_ptr<s2s::mysql::response>
			response = std::make_shared<s2s::mysql::response>();
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
	XCode MysqlDataComponent::Delete(const std::string& deleteJson, long long flag)
	{
        T data;
        return this->Delete(data.GetTypeName(), deleteJson, flag);
	}

	template<typename T>
	XCode MysqlDataComponent::Update(const std::string& updateJson, const std::string& whereJson, long long flag)
	{
        T data;
        return this->Update(data.GetTypeName(), whereJson, updateJson, flag);
	}
}
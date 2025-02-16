
#include "Mysql/Client/Client.h"
#include "Mysql/Common/MysqlProto.h"
#include "Rpc/Component/RpcComponent.h"

namespace acs
{
	class MysqlTask final : public IRpcTask<mysql::Response>, protected WaitTaskSourceBase
	{
	public:
		explicit MysqlTask(int taskId);
	public:
		inline std::unique_ptr<mysql::Response> Await();
		inline void OnResponse(std::unique_ptr<mysql::Response> response) noexcept final;
	private:
		std::unique_ptr<mysql::Response> mMessage;
	};
	inline std::unique_ptr<mysql::Response> MysqlTask::Await()
	{
		this->YieldTask();
		return std::move(this->mMessage);
	}
	inline void MysqlTask::OnResponse(std::unique_ptr<mysql::Response> response) noexcept
	{
		this->mMessage = std::move(response);
		this->ResumeTask();
	}
}

namespace acs
{
	class MysqlDBComponent : public RpcComponent<mysql::Response>,
							 public IRpc<mysql::Request, mysql::Response>
	{
	public:
		MysqlDBComponent();
	public:
		void Send(std::unique_ptr<mysql::Request> request);
		void Send(std::unique_ptr<mysql::Request> request, int & rpcId);
		std::unique_ptr<mysql::Response> Run(std::unique_ptr<mysql::Request> request);
	private:
		void Send(int id, std::unique_ptr<mysql::Request> request);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnMessage(int id, mysql::Request *request, mysql::Response *response) noexcept final;
	private:
		mysql::Config mConfig;
		custom::Queue<int> mFreeClients;
		std::queue<std::unique_ptr<mysql::Request>> mMessages;
		std::unordered_map<int, std::shared_ptr<mysql::Client>> mClients;
	};
}

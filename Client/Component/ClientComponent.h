#pragma once
#include<Component/Component.h>

using namespace GameKeeper;
using namespace google::protobuf;
namespace Client
{
	class ClientRpcTask;
	class ClientComponent : public Component,
		public IRpc<c2s::Rpc_Request, c2s::Rpc_Response>
	{
	public:
		ClientComponent();
		~ClientComponent() final = default;
	public:
		void StartClose(long long id);
		void OnRequest(c2s::Rpc_Request * request);
		void OnResponse(c2s::Rpc_Response * response);		
		void OnCloseSocket(long long id, XCode code);
		void OnConnectAfter(long long id, XCode code) { }
	public:
		unsigned int AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms);
	protected:
		bool Awake() override;
		void Start() override;
		void OnTimeout(long long rpcId);
	private:
		class TcpRpcClient * mTcpClient;
		class TimerComponent * mTimerComponent;
		class TaskPoolComponent * mTaskComponent;
		std::unordered_map<long long, std::shared_ptr<ClientRpcTask>> mRpcTasks;
	};
}
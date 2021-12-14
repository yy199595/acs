#pragma once
#include<Component/Component.h>

using namespace GameKeeper;
using namespace google::protobuf;
namespace Client
{
	class ClientRpcTask;
	class ClientComponent : public Component,
                            public IRpc<c2s::Rpc_Request, c2s::Rpc_Response>, public IStart
	{
	public:
		ClientComponent();
		~ClientComponent() final = default;
	public:
		void StartClose(long long id) final;
		void OnRequest(c2s::Rpc_Request * request) final;
		void OnResponse(c2s::Rpc_Response * response) final;
		void OnCloseSocket(long long id, XCode code) final;
		void OnConnectAfter(long long id, XCode code) final { }
	public:
		unsigned int AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms);
	protected:
		bool Awake() final;
        void OnStart() final;
		bool LateAwake() final;
		void OnTimeout(long long rpcId);
	private:
		class TcpRpcClient * mTcpClient;
		class TimerComponent * mTimerComponent;
		class ThreadPoolComponent * mTaskComponent;
		std::unordered_map<long long, std::shared_ptr<ClientRpcTask>> mRpcTasks;
	};
}
#pragma once
#include<string>
#include"Protocol/c2s.pb.h"
#include"Async/Task.h"
#include<google/protobuf/message.h>


using namespace GameKeeper;
using namespace google::protobuf;
namespace Client
{
    class ClientRpcTask : public std::enable_shared_from_this<ClientRpcTask>
	{
	public:
		ClientRpcTask(const std::string & method);
	public:
		void OnResponse(const c2s::Rpc_Response * response);
		long long GetRpcTaskId() const { return this->mRpcId; }
	public:
        XCode GetCode();
        template<typename T>
		std::shared_ptr<T> GetData();

    private:
        bool YieldTask();
	private:
		XCode mCode;
		int mTimeout;
        long long mRpcId;
        TaskState mState;
        unsigned int mTimerId;
		long long mStartTime;
        unsigned int mCoroutineId;
		const std::string mMethod;
        TaskComponent * mTaskComponent;
        std::shared_ptr<Message> mMessage;
        class ClientComponent * mClientComponent;
    };

    template<typename T>
    std::shared_ptr<T> ClientRpcTask::GetData()
    {
        this->YieldTask();
        return dynamic_pointer_cast<T>(mMessage);
    }
}
#pragma once
#include<string>
#include"Async/AsyncTask.h"
#include"Protocol/c2s.pb.h"
#include<google/protobuf/message.h>


using namespace GameKeeper;
using namespace google::protobuf;
namespace Client
{
	class ClientRpcTask : public AsyncTask, public std::enable_shared_from_this<ClientRpcTask>
	{
	public:
		ClientRpcTask(const std::string & method, long long rpcId);
	public:
		void OnResponse(const c2s::Rpc_Response * resoinse);
		long long GetRpcTaskId() const { return this->mRpcId; }
	public:
		XCode AwaitGetCode(int ms = 5000);
		template<typename T>
		std::shared_ptr<T> AwaitGetData(int ms = 5000);
	protected:
		void OnTaskAwait() final;
	private:
		XCode mCode;
		int mTimeout;
		unsigned int mTimerId;
		long long mStartTime;
		const long long mRpcId;
		const std::string mMethod;
		std::shared_ptr<Message> mMessage;
		
	};
	template<typename T>
	inline std::shared_ptr<T> ClientRpcTask::AwaitGetData(int ms)
	{
		this->mTimeout = ms;
		this->AsyncAwaitTask();
		return std::dynamic_pointer_cast<T>(this->mMessage);
	}
}
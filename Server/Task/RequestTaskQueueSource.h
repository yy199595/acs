//
// Created by yjz on 2022/4/17.
//

#ifndef _REQUESTTASKQUEUESOURCE_H_
#define _REQUESTTASKQUEUESOURCE_H_
#include"Async/TaskSource.h"
#include"Message/c2s.pb.h"
namespace Sentry
{
	class RequestTaskQueueSource : public WaitTaskSourceBase
	{
	 public:
		std::shared_ptr<c2s::Rpc::Request> Await();
		void AddResult(std::shared_ptr<c2s::Rpc::Request> & data);
	 private:
		std::shared_ptr<c2s::Rpc::Request> mRequest;
		std::queue<std::shared_ptr<c2s::Rpc::Request>> mTaskQueue;
	};
}

#endif //_REQUESTTASKQUEUESOURCE_H_

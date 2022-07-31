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
		std::shared_ptr<c2s::rpc::request> Await();
		void AddResult(std::shared_ptr<c2s::rpc::request> & data);
	 private:
		std::shared_ptr<c2s::rpc::request> mRequest;
		std::queue<std::shared_ptr<c2s::rpc::request>> mTaskQueue;
	};
}

#endif //_REQUESTTASKQUEUESOURCE_H_

#pragma once
#include<thread>
#include"TaskThread.h"
#include"ThreadTaskAction.h"
namespace SoEasy
{
	class ThreadPool
	{
	public:	
		ThreadPool(unsigned int count = 0);	//启动线程池数量
		TaskThread * GetTaskThread(long long id);
		void GetAllTaskThread(std::vector<long long> & threads);
		bool StartTaskAction(std::shared_ptr<ThreadTaskAction> taskAction);
	public:	
		size_t GetThreadCount() { return mThreadMap.size(); }
		static unsigned int GetConcurrencyCount() { return thread::hardware_concurrency(); }
	private:
		std::unordered_map<long long, TaskThread *> mThreadMap;
	};
}
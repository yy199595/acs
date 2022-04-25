//
// Created by mac on 2022/4/25.
//

#ifndef SERVER_COROUTINELOCK_H
#define SERVER_COROUTINELOCK_H
#include<queue>
#include"Async/TaskSource.h"
namespace Sentry
{
	class CoroutineLock
	{
	public:
		CoroutineLock();

	public:
		void Lock();

		void UnLock();

	private:
		bool mIsLock;
		std::queue<unsigned int> mWaitTasks;

		class TaskComponent* mTaskComponent;
	};

	class AutoCoroutineLock
	{
	public:
		AutoCoroutineLock(std::shared_ptr<CoroutineLock> lock)
				: mLock(lock)
		{
			this->mLock->Lock();
		}

		~AutoCoroutineLock()
		{
			this->mLock->UnLock();
		}

	private:
		std::shared_ptr<CoroutineLock> mLock;
	};
}


#endif //SERVER_COROUTINELOCK_H

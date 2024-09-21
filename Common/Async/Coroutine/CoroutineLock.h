//
// Created by mac on 2022/4/25.
//

#ifndef SERVER_COROUTINELOCK_H
#define SERVER_COROUTINELOCK_H
#include<queue>
#include <utility>
#include"Async/Source/TaskSource.h"
namespace acs
{
	class CoroutineComponent;
	class CoroutineLock
	{
	public:
		CoroutineLock(CoroutineComponent * cor);
	public:
		void Lock();
		void UnLock();
		bool IsLock() const { return this->mIsLock;}
	private:
		bool mIsLock;
		CoroutineComponent * mCoroutine;
		std::queue<unsigned int> mWaitTasks;
	};

	class AutoCoroutineLock
	{
	public:
		explicit AutoCoroutineLock(CoroutineLock & lock)
				: mLock(lock)
		{
			this->mLock.Lock();
		}

		~AutoCoroutineLock()
		{
			this->mLock.UnLock();
		}

	private:
		CoroutineLock & mLock;
	};
}


#endif //SERVER_COROUTINELOCK_H

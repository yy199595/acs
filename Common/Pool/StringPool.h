//
// Created by zmhy0073 on 2021/10/20.
//

#ifndef SENTRY_STRINGPOOL_H
#define SENTRY_STRINGPOOL_H

#include <string>
#include <atomic>
#ifdef __THREAD_LOCK__
#include<queue>
#include<mutex>
#else
#include<Define/ThreadQueue.h>
#endif
namespace Sentry
{
    class StringPool
    {
    public:
        StringPool(size_t size = 100);
        ~StringPool();
    public:
        std::string * New();
        std::string * New(const char * str);
        std::string * New(const char * str, size_t size);
        int GetSize() { return this->mCurrentSize; }
    public:
        void Destory(std::string * str);
    private:
        const size_t mMaxSize;
        std::atomic_int mCurrentSize;
#ifdef __THREAD_LOCK__
        std::mutex mLock;
        std::queue<std::string *> mStringQueue;
#else
        MultiThread::ConcurrentQueue<std::string *> mStringQueue;
#endif
    };
}

#endif //SENTRY_STRINGPOOL_H

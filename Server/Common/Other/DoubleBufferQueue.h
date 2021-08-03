#pragma once
#pragma once

#include<queue>
#include<mutex>
#include<atomic>

namespace Sentry
{
    template<typename T>
    class DoubleBufferQueue
    {
    public:
        DoubleBufferQueue() {}

    public:
        void AddItem(const T &item);

        bool PopItem(T &item);

        void SwapQueueData();

    private:
        std::mutex mLock;
    private:
        std::queue<T> mReadQueue;
        std::queue<T> mWrterQueue;
    };

    template<typename T>
    inline void DoubleBufferQueue<T>::AddItem(const T &item)
    {
        mLock.lock();
        mWrterQueue.emplace(item);
        mLock.unlock();
    }

    template<typename T>
    inline void DoubleBufferQueue<T>::SwapQueueData()
    {
        if (!this->mWrterQueue.empty() && this->mReadQueue.empty())
        {
            mLock.lock();
            //std::swap(this->mCacheQueue, this->mWorkQueue);
            this->mReadQueue.swap(this->mWrterQueue);
            mLock.unlock();
        }

    }

    template<typename T>
    inline bool DoubleBufferQueue<T>::PopItem(T &item)
    {
        if (!this->mReadQueue.empty())
        {
            item = this->mReadQueue.front();
            this->mReadQueue.pop();
            return true;
        }
        return false;
    }
}
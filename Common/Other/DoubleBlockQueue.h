//
// Created by yjz on 2022/1/6.
//

#ifndef GAMEKEEPER_DOUBLEBLOCKQUEUE_H
#define GAMEKEEPER_DOUBLEBLOCKQUEUE_H

#endif //GAMEKEEPER_DOUBLEBLOCKQUEUE_H
#include<queue>
#include<mutex>
namespace GameKeeper
{
    template<typename T>
    class DoubleBlockQueue
    {
    public:
        void Swap();
        void Push(T & value);
        bool Pop(T & value);
    private:
        std::mutex mLock;
        std::queue<T> mReadQueue;
        std::queue<T> mWriteQueue;
    };
    template<typename T>
    void DoubleBlockQueue<T>::Push(T &value)
    {
        this->mLock.lock();
        this->mWriteQueue.emplace(value);
        this->mLock.unlock();
    }

    template<typename T>
    bool DoubleBlockQueue<T>::Pop(T &value)
    {
        if(this->mReadQueue.empty())
        {
            return false;
        }
        value = this->mReadQueue.front();
        this->mReadQueue.pop();
        return true;
    }

    template<typename T>
    void DoubleBlockQueue<T>::Swap()
    {
        this->mLock.lock();
        if(this->mReadQueue.empty())
        {
            std::swap(this->mReadQueue, this->mWriteQueue);
        }
        this->mLock.unlock();
    }
}

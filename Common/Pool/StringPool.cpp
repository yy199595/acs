//
// Created by zmhy0073 on 2021/10/20.
//

#include "StringPool.h"
#include <thread>

namespace GameKeeper
{
    StringPool::StringPool(size_t size)
        : mMaxSize(size)
    {
        this->mCurrentSize = 0;
        for (size_t index = 0; index < size; index++)
        {
            this->mCurrentSize++;
#ifdef __THREAD_LOCK__
            this->mStringQueue.push(new std::string());
#else
            this->mStringQueue.enqueue(new std::string());
#endif
        }
    }

    StringPool::~StringPool()
    {
#ifdef __THREAD_LOCK__
        std::lock_guard<std::mutex> lockGuard(this->mLock);
        while(!this->mStringQueue.empty())
        {
            delete this->mStringQueue.front();
            this->mStringQueue.pop();
        }
#else
        std::string * str = nullptr;
        while(this->mStringQueue.try_dequeue(str))
        {
            delete str;
            this->mCurrentSize--;
        }
#endif
    }

    std::string *StringPool::New()
    {
        std::string *str = nullptr;
        if (this->mCurrentSize <= 0)
        {
            return new std::string();
        }
#ifdef __THREAD_LOCK__
        std::lock_guard<std::mutex> lockGuard(this->mLock);
        str = this->mStringQueue.front();
        this->mStringQueue.pop();
        this->mCurrentSize--;
#else
        if (this->mStringQueue.try_dequeue(str))
        {
            this->mCurrentSize--;
        }
#endif
        return str;
    }

    std::string *StringPool::New(const char *str)
    {
        std::string * data = this->New();
        data->append(str);
        return data;
    }

    std::string *StringPool::New(const char *str, size_t size)
    {
        std::string * data = this->New();
        data->append(str, size);
        return data;
    }

    void StringPool::Destory(std::string *str)
    {
        if (str == nullptr)
        {
            return;
        }
        if (this->mCurrentSize >= this->mMaxSize)
        {
            delete str;
        }
        else
        {
            this->mCurrentSize++;
#ifdef __THREAD_LOCK__
            std::lock_guard<std::mutex> lockGuard(this->mLock);
            str->clear();
            this->mStringQueue.push(str);

#else
            this->mStringQueue.enqueue(str);
#endif
        }
    }
}
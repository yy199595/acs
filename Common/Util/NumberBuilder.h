#pragma once
#include <Define/CommonTypeDef.h>
namespace Sentry
{
    template<typename T>
    class NumberBuilder
    {
    public:
        NumberBuilder(const T num = 100) : mIndex(num) {}
        inline const T Pop();
        inline void Push(const T num);

    private:
        T mIndex;
        std::queue<T> mNumberQueue;
    };
    template<typename T>
    inline const T NumberBuilder<T>::Pop()
    {
        if (!this->mNumberQueue.empty())
        {
            T number = this->mNumberQueue.front();
            this->mNumberQueue.pop();
            return number;
        }
        return this->mIndex++;
    }
    template<typename T>
    inline void NumberBuilder<T>::Push(const T num)
    {
        this->mNumberQueue.push(num);
    }
}// namespace Sentry
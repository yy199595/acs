#pragma once
#include "Define/CommonTypeDef.h"
namespace Util
{
    template<typename T, int Start>
    class NumberBuilder
    {
    public:
        explicit NumberBuilder() : mIndex(Start) {}
        inline const T Pop();
        inline void Push(T num);

    private:
        T mIndex;
        std::queue<T> mNumberQueue;
    };
    template<typename T, int Start>
    inline const T NumberBuilder<T, Start>::Pop()
    {
        if (!this->mNumberQueue.empty())
        {
            T number = this->mNumberQueue.front();
            this->mNumberQueue.pop();
            return number;
        }
        return this->mIndex++;
    }
    template<typename T, int Start>
    inline void NumberBuilder<T, Start>::Push(const T num)
    {
        this->mNumberQueue.push(num);
    }
}// namespace Sentry
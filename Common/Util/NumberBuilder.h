#pragma once
#include <Define/CommonTypeDef.h>
namespace GameKeeper
{
    template<typename T>
    class NumberBuilder
    {
    public:
        explicit NumberBuilder(const T num = 1) : mIndex(num) {}
        inline const T Pop();
        inline void Push(T num);

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
}// namespace GameKeeper
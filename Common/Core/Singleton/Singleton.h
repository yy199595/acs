//
// Created by zmhy0073 on 2022/10/14.
//

#ifndef APP_SINGLETON_H
#define APP_SINGLETON_H
#include<cassert>

namespace acs
{
    template<typename T>
    class Singleton
    {
    public:
        Singleton()
        {
            assert(mInstance == nullptr);
            mInstance = (T*)this;
        }
        static inline T * Inst() { return Singleton<T>::mInstance; }
    private:
        static T * mInstance;
    };


    template<typename T>
    class ConstSingleton
    {
    public:
        ConstSingleton()
        {
            assert(mInstance == nullptr);
            mInstance = (T*)this;
        }
        static const T * Inst() { return ConstSingleton<T>::mInstance; }
    public:
        static const T * mInstance;
    };
}

namespace acs
{
    template<typename T>
    T * Singleton<T>::mInstance = nullptr;
    template<typename T>
    const T * ConstSingleton<T>::mInstance = nullptr;
}

#endif //APP_SINGLETON_H
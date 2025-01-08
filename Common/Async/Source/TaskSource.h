//
// Created by yjz on 2022/1/5.
//

#ifndef APP_TASKSOURCE_H
#define APP_TASKSOURCE_H
#include"WaitTaskSourceBase.h"
namespace acs
{
    class LoopTaskSource final : public WaitTaskSourceBase
    {
    public:
        void Await() {this->YieldTask();}
        bool SetResult() { return this->ResumeTask(TaskState::TaskReady);}
    };

    template<typename T>
    class TaskSource final : public WaitTaskSourceBase
    {
    public:
        const T & Await();
        bool SetResult(const T & data);
    private:
        T mData;
    };

    template<typename T>
    const T & TaskSource<T>::Await()
    {
        this->YieldTask();
        return this->mData;
    }

    template<typename T>
    bool TaskSource<T>::SetResult(const T &data)
    {
        if(this->ResumeTask())
        {
            this->mData = data;
            return true;
        }
        return false;
    }
}

namespace acs
{
    template<> class TaskSource<void> : public WaitTaskSourceBase
    {
    public:
        void Await() noexcept { this->YieldTask();}
        bool SetResult() noexcept { return this->ResumeTask();}
    };
}

namespace acs
{
    template<typename T>
    class TaskSource<T *> { };
    template<typename T>
    class TaskSource<T &> { };
    template<typename T>
    class TaskSource<const T *> { };
    template<typename T>
    class TaskSource<const T &> { };
}

namespace acs
{
//    template<typename T>
//    class TaskSource<std::shared_ptr<T>> : public WaitTaskSourceBase
//    {
//    public:
//        std::shared_ptr<T> Await();
//        bool SetResult(const std::shared_ptr<T> & data);
//    private:
//        std::shared_ptr<T> mData;
//    };
//    template<typename T>
//    std::shared_ptr<T> TaskSource<std::shared_ptr<T>>::Await()
//    {
//        this->YieldTask();
//        return std::move(this->mData);
//    }
//    template<typename T>
//    bool TaskSource<std::shared_ptr<T>>::SetResult(const std::shared_ptr<T> & data)
//    {
//		this->mData = std::move(data);
//		return this->ResumeTask();
//    }
}

#endif //APP_TASKSOURCE_H

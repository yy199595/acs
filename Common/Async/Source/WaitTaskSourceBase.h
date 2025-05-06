//
// Created by zmhy0073 on 2021/11/29.
//

#ifndef APP_WAITTASKSOURCEBASE_H
#define APP_WAITTASKSOURCEBASE_H
#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif
namespace acs
{
    enum class TaskState
    {
        TaskReady,
        TaskAwait,
        TaskFinish,
    };

    class WaitTaskSourceBase
#ifdef __SHARE_PTR_COUNTER__
        : public memory::Object<WaitTaskSourceBase>
#endif
    {
    public:
        explicit WaitTaskSourceBase();
#ifdef __SHARE_PTR_COUNTER__
        ~WaitTaskSourceBase() override = default;
#else
        virtual ~WaitTaskSourceBase() = default;
#endif
    public:
		void Clear();
        inline TaskState GetState() const { return this->mState; }
        inline bool IsComplete() { return this->mState == TaskState::TaskFinish;}
    protected:
        bool YieldTask() noexcept;
        bool ResumeTask(TaskState state = TaskState::TaskFinish) noexcept;
    private:
        TaskState mState;
        unsigned int mCorId;
		class CoroutineComponent * mCoroutine;
    };
}

#endif //APP_WAITTASKSOURCEBASE_H

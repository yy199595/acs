//
// Created by zmhy0073 on 2021/11/29.
//

#ifndef APP_WAITTASKSOURCEBASE_H
#define APP_WAITTASKSOURCEBASE_H
#include<memory>
namespace acs
{
    enum class TaskState
    {
        TaskReady,
        TaskAwait,
        TaskFinish,
    };

    class WaitTaskSourceBase
    {
    public:
        explicit WaitTaskSourceBase();
        virtual ~WaitTaskSourceBase() = default;
    public:
		void Clear();
        TaskState GetState() const { return this->mState; }
        bool IsComplete() { return this->mState == TaskState::TaskFinish;}
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

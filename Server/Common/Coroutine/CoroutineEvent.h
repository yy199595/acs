#pragma once

#include<functional>

#ifdef _WIN32
#define STACK_SIZE 1024 * 2
#else
#define STACK_SIZE 1024 * 1024
#endif
namespace Sentry
{
    class CoroutineManager;

    typedef std::function<void()> CoroutineAction;

    class CoroutineEvent
    {
    public:
        CoroutineEvent(CoroutineManager *mgr, long long id);

    public:
        virtual bool Invoke() = 0;

    public:
        long long GetBindCoroutineID() { return mBindCoroutineId; }

    protected:
        CoroutineManager *mCorScheduler;
    private:
        long long mBindCoroutineId;
    };

    class CorSleepEvent : public CoroutineEvent
    {
    public:
        CorSleepEvent(CoroutineManager *, long long id, long long ms);

    public:
        bool Invoke() override;

    private:
        long long mNextInvokeTime;
    };

    class CorNextFrameEvent : public CoroutineEvent
    {
    public:
        CorNextFrameEvent(CoroutineManager *, long long id, int count);

    public:
        bool Invoke() override;

    private:
        int mCount;
        int mMaxCount;
    };
}
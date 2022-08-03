#pragma once
#include <string>
#include"CoroutineDef.h"


namespace Method
{
    class IFucntion
    {
    public:
        inline virtual void Invoke() = 0;
    };

    template<typename T, typename ... Args>
    class Function : public IFucntion
    {
    public:
        typedef void(T::*ClassFunc)(Args ...);
        Function(T * o, ClassFunc && func, Args && ... t1)
            : mFunc(std::bind(func, o, std::forward<Args>(t1)...)) { }

        Function(std::shared_ptr<T> o, ClassFunc && func, Args && ... t1)
            : mFunc(std::bind(func, o, std::forward<Args>(t1) ...)) { }
        Function(const Function<T, Args...> & obj) { this->mFunc = obj.mFunc; }
    public:
        inline void Invoke() final { this->mFunc(); }
    private:
        std::function<void(Args ...)> mFunc;
    };

    template<> class Function<void> : public IFucntion
    {
    public:
        Function(std::function<void(void)> && func) : mFunc(std::move(func)) { }
        Function(const Function<void> & obj) { this->mFunc = obj.mFunc; }
    public:
        inline void Invoke() final { this->mFunc(); }
    private:
        std::function<void(void)> mFunc;
    };

}

namespace Sentry
{
	enum class CorState
	{
		Ready,
		Running,
		Suspend,
		Finish,
	};

	class TaskComponent;

	struct Stack
	{
		char* p;
		char* top;
		size_t size = 0;
		unsigned int co = 0;
	};
	class CoroutineGroup;

	class TaskContext
	{
	 public:
		explicit TaskContext();
		~TaskContext();
	 public:
		void Invoke();
	 public:
		int sid;
        Stack mStack;
        CorState mState;
        tb_context_t mContext;
		StaticMethod* mFunction;
		unsigned int mSwitchCount;
		unsigned int mCoroutineId;
        std::shared_ptr<CoroutineGroup> mGroup;
    };
}
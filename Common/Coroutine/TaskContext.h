#pragma once
#include <string>
#include"CoroutineDef.h"
namespace Sentry
{
	enum CorState
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
		CorState mState;
#ifdef __COROUTINE_BUFFER_STRING__
        std::string mStack;
#else
        Stack mStack;
#endif
        tb_context_t mContext;
		CoroutineGroup* mGroup;
		StaticMethod* mFunction;
		unsigned int mSwitchCount;
		unsigned int mCoroutineId;
	};
}
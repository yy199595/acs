#pragma once
#include <string>
#include"CoroutineDef.h"
#include"Rpc/Method/MethodProxy.h"

namespace Tendo
{
	enum class CorState
	{
		Ready,
		Running,
		Suspend,
		Finish,
	};

	class CoroutineComponent;

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
#ifdef 	__COR_SHARED_STACK__
		int sid;
#endif
        Stack mStack;
        CorState mState;
        tb_context_t mContext;
		StaticMethod* mFunction;
		unsigned int mCoroutineId;
        std::shared_ptr<CoroutineGroup> mGroup;
    };
}
#pragma once
#include <string>
#include"CoroutineDef.h"
#include"Rpc/Method/MethodProxy.h"

#ifdef __MEMORY_POOL_OPERATOR__
#include <vector>
#endif

#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif
namespace acs
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
		char* p = nullptr;
		char* top = nullptr;
		size_t size = 0;
		unsigned int co = 0;
	};

	class TaskContext
#ifdef __SHARE_PTR_COUNTER__
	: public memory::Object<TaskContext>
#endif
	{
	 public:
		explicit TaskContext();
#ifdef __SHARE_PTR_COUNTER__
		~TaskContext() final;
#else
		~TaskContext();
#endif

#ifdef __MEMORY_POOL_OPERATOR__
	public:
		void * operator new(size_t size);
		void operator delete (void * ptr);
#endif
	 public:
		void Invoke();
	 public:
		int sid;
        Stack mStack;
        CorState mState;
        tb_context_t mContext;
		unsigned int mCoroutineId;
		std::unique_ptr<StaticMethod> mFunction;
#ifdef __MEMORY_POOL_OPERATOR__
	private:
		static std::vector<void *> sAllocArray;
#endif
    };
}
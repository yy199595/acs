#pragma once
#include <string>
#include "Rpc/Method/MethodProxy.h"
#include "Async/Coroutine/Context/context.h"

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
		size_t size = 0;
#ifdef __ENABLE_SHARE_STACK__
		unsigned int co = 0;
		const char* top = nullptr;
#endif
	};

	class TaskContext final
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

	public:
		void Invoke();
	public:
		Stack stack;
		unsigned int id;
#ifdef __ENABLE_SHARE_STACK__
		unsigned int sid;
#endif
		CorState status;
		long long timerId;
		tb_context_t ctx;
		std::unique_ptr<StaticMethod> cb;
	};
}
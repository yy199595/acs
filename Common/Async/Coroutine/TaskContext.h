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
		int sid;
        Stack stack;
		unsigned int id;
        CorState status;
        tb_context_t mContext;
		std::unique_ptr<StaticMethod> callback;
    };
}
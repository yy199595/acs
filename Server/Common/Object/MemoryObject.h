#pragma once
#include<Other/MemoryPool.h>
namespace Sentry
{
	class MemoryObject
	{
	public:

	private:
		const size_t mClassSize;
		static MemoryPool * mMmoryPool;
	};
}
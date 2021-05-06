#pragma once
#include<Other/MemoryPool.h>
namespace SoEasy
{
	class MemoryObject
	{
	public:

	private:
		const size_t mClassSize;
		static MemoryPool * mMmoryPool;
	};
}
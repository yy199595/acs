//
// Created by yjz on 2022/5/15.
//

#ifndef _PROTOPOOL_H_
#define _PROTOPOOL_H_
#include<queue>
namespace Sentry
{
	template<typename T, size_t MaxSize>
	class ProtoPool
	{
	 public:
		std::shared_ptr<T> Pop();
		void Push(std::shared_ptr<T> data);
	 private:
		std::queue<std::shared_ptr<T>> mPool;
	};

	template<typename T, size_t MaxSize>
	std::shared_ptr<T> ProtoPool<T, MaxSize>::Pop()
	{
		if(!this->mPool.empty())
		{
			std::shared_ptr<T> data = this->mPool.front();
			this->mPool.pop();
			return data;
		}
		return std::make_shared<T>();
	}
	template<typename T, size_t MaxSize>
	void ProtoPool<T, MaxSize>::Push(std::shared_ptr<T> data)
	{
		if(this->mPool.size() < MaxSize)
		{
			data->Clear();
			this->mPool.push(data);
		}
	}
}

#endif //_PROTOPOOL_H_

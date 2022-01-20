//
// Created by zmhy0073 on 2021/11/22.
//

#ifndef GAMEKEEPER_ALLOTORPOOL_H
#define GAMEKEEPER_ALLOTORPOOL_H
#include<queue>

namespace Sentry
{
    template<typename T>
    class AllotorPool
    {
    public:
        explicit AllotorPool(size_t size = 100) : mMaxCount(size) { }
        ~AllotorPool()
        {
            while(!this->mMemoryPool.empty())
            {
                T *data = (T *) this->mMemoryPool.front();
                this->mMemoryPool.pop();
                delete data;
            }
        }
    public:
        template<typename ... Args>
        T * New(Args  && ... args)
        {
            if (this->mMemoryPool.empty())
            {
                return new T(std::forward<Args>(args) ...);
            }
            void *mem = this->mMemoryPool.front();
            this->mMemoryPool.pop();
            return new(mem)T(std::forward<Args>(args)...);
        }
        void Delete(T * object)
        {
            if(object == nullptr)
            {
                return;
            }
            if(this->mMemoryPool.size() >= mMaxCount)
            {
                delete object;
                return;
            }
            object->Clear();
            this->mMemoryPool.push(object);
        }

    private:
        const size_t mMaxCount;
        std::queue<void *> mMemoryPool;
    };


}
#endif //GAMEKEEPER_ALLOTORPOOL_H

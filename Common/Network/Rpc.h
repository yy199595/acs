//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef GameKeeper_MESSAGESTREAM_H
#define GameKeeper_MESSAGESTREAM_H
#include <Define/CommonDef.h>
#include <Define/CommonTypeDef.h>
#include <Protocol/com.pb.h>
#include <Protocol/c2s.pb.h>
#include <queue>

#define RPC_TYPE_REQUEST 1
#define RPC_TYPE_RESPONSE 2
#define PROTO_POOL_COUNT 1024

namespace GameKeeper
{
    template<typename T>
    class ProtoPool
    {
    public:
        ProtoPool() = default;
    public:
        T * Create();
        void Destory(T * data);
    private:
        std::queue<T *> mPool;
    };

    template<typename T>
    T *ProtoPool<T>::Create()
    {
        if(!this->mPool.empty())
        {
            T * message = this->mPool.front();
            this->mPool.pop();
            return message;
        }
        return new T();
    }

    template<typename T>
    void ProtoPool<T>::Destory(T *data)
    {
        if(data != nullptr && this->mPool.size() < PROTO_POOL_COUNT)
        {
            data->Clear();
            this->mPool.push(data);
            return;
        }
        delete data;
    }
}

namespace GameKeeper
{
    extern thread_local
    extern thread_local ProtoPool<com::Rpc_Request> RequestPool;
    extern thread_local ProtoPool<com::Rpc_Response> ResponsePool;
}

#endif //GameKeeper_MESSAGESTREAM_H

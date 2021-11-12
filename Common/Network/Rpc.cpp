//
// Created by zmhy0073 on 2021/11/12.
//

#include "Rpc.h"
namespace GameKeeper
{
    thread_local ProtoPool<com::Rpc_Request> RequestPool;
    thread_local ProtoPool<com::Rpc_Response> ResponsePool;
}
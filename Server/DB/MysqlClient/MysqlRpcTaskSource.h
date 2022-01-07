//
// Created by zmhy0073 on 2021/11/29.
//

#ifndef GAMEKEEPER_MYSQLRPCTASKSOURCE_H
#define GAMEKEEPER_MYSQLRPCTASKSOURCE_H
#include<memory>
#include"XCode/XCode.h"
#include"Protocol/db.pb.h"
#include"Async/RpcTask/RpcTaskSource.h"
using namespace google::protobuf;
namespace GameKeeper
{
    class RpcComponent;
    class MysqlRpcTaskSource : public IRpcTask
    {
    public:
        explicit MysqlRpcTaskSource(int timeout = 0)
            : mTimeout(timeout) { }
        ~MysqlRpcTaskSource() = default;
    public:
        XCode GetCode();
        size_t GetDataSize();
        template<typename T>
        std::shared_ptr<T> GetData(size_t index = 0);
    public:
        int GetTimeout() final { return this->mTimeout;}
        long long GetRpcId() final { return mTaskSource.GetTaskId();}
        void OnResponse(std::shared_ptr<com::Rpc_Response> response) final;
    private:
        XCode mCode;
        int mTimeout;
        RpcComponent * mRpcComponent;
        TaskSource<std::shared_ptr<s2s::MysqlResponse>> mTaskSource;
    };

    template<typename T>
    std::shared_ptr<T> MysqlRpcTaskSource::GetData(size_t index)
    {
        auto response = this->mTaskSource.Await();
        if(response == nullptr || index < 0 || response->datas_size()
           || index >= response->datas_size())
        {
            return nullptr;
        }
        std::shared_ptr<T> data(new T());
        const Any & any = response->datas(index);
        if(any.Is<T>() && any.UnpackTo(data.get()))
        {
            return data;
        }
        return nullptr;
    }
}


#endif //GAMEKEEPER_MYSQLRPCTASKSOURCE_H

﻿#pragma once

#include<memory>
#include<Method/SubMethod.h>
#include<Component/Component.h>
#include<Method/ServiceMethod.h>

using namespace std;
using namespace com;

namespace Sentry
{
    class ServiceMethod;
    class ServiceComponentBase : public Component {
    public:
        ServiceComponentBase() = default;

        ~ServiceComponentBase() override = default;

    public:

        virtual const std::string & GetServiceName() { return this->GetTypeName();}

        std::shared_ptr<com::Rpc_Response> Invoke(const std::string &method, std::shared_ptr<com::Rpc_Request> request);

    public:
        bool AddMethod(ServiceMethod *method);

        void Publish(const std::string & func, const std::string & message);

        void GetSubMethods(std::vector<std::string> & methods);

        long long GetCurSocketId() const { return this->mCurSocketId; }

        void SetCurSocketId(long long socketId) { this->mCurSocketId = socketId; }

    protected:
        template<typename T>
        bool Bind(std::string name, ServiceMethodType1<T> func)
        {
            return this->AddMethod(new ServiceMethod1<T>(name, (T *) this, func));
        }

        template<typename T>
        bool Bind(std::string name, ServiceMethodType11<T> func)
        {
            return this->AddMethod(new ServiceMethod1<T>(name, (T *) this, func));
        }

        template<typename T, typename T1>
        bool Bind(std::string name, ServiceMethodType2<T, T1> func)
        {
            return this->AddMethod(new ServiceMethod2<T, T1>(name, (T *) this, func));
        }

        template<typename T, typename T1>
        bool Bind(std::string name, ServiceMethodType22<T, T1> func)
        {
            return this->AddMethod(new ServiceMethod2<T, T1>(name, (T *) this, func));
        }

        template<typename T, typename T1, typename T2>
        bool Bind(std::string name, ServiceMethodType3<T, T1, T2> func)
        {
            return this->AddMethod(new ServiceMethod3<T, T1, T2>(name, (T *) this, func));
        }

        template<typename T, typename T1, typename T2>
        bool Bind(std::string name, ServiceMethodType33<T, T1, T2> func)
        {
            return this->AddMethod(new ServiceMethod3<T, T1, T2>(name, (T *) this, func));
        }

        template<typename T, typename T1>
        bool Bind(std::string name, ServiceMethodType4<T, T1> func)
        {
            return this->AddMethod(new ServiceMethod4<T, T1>(name, (T *) this, func));
        }

        template<typename T, typename T1>
        bool Bind(std::string name, ServiceMethodType44<T, T1> func)
        {
            return this->AddMethod(new ServiceMethod4<T, T1>(name, (T *) this, func));
        }

        template<typename T>
        bool Sub(std::string name, JsonSubFunction<T> func)
        {
            auto iter = this->mSubMethodMap.find(name);
            if(iter != this->mSubMethodMap.end())
            {
                return false;
            }
            JsonSubMethod<T> * jsonSubMethod = new JsonSubMethod<T>((T*)this, func);
            this->mSubMethodMap.emplace(name, jsonSubMethod);
            return true;
        }

        template<typename T, typename T1>
        bool Sub(std::string name, ProtoSubFuncrion<T,T1> func)
        {
            auto iter = this->mSubMethodMap.find(name);
            if(iter != this->mSubMethodMap.end())
            {
                return false;
            }
            ProtoSubMethod<T, T1> * subMethod = new ProtoSubMethod<T, T1>((T*)this, func);
            this->mSubMethodMap.emplace(name, subMethod);
            return true;
        }

    private:
        ServiceMethod * GetMethod(const std::string & name);

    private:
        long long mCurSocketId;
        std::unordered_map<std::string, SubMethod *> mSubMethodMap;
        std::unordered_map<std::string, ServiceMethod *> mMethodMap;
        std::unordered_map<std::string, ServiceMethod *> mLuaMethodMap;
    };

    inline std::string GetFunctionName(const std::string func)
    {
        size_t pos = func.find("::");
        return func.substr(pos + 2);
    }

#define BIND_SUB_FUNCTION(func) LOG_CHECK_RET_FALSE(this->Sub(GetFunctionName(#func), &func))
#define BIND_RPC_FUNCTION(func) LOG_CHECK_RET_FALSE(this->Bind(GetFunctionName(#func), &func))

}
//
// Created by yjz on 2022/1/22.
//
#include"Component.h"
#include"Method/SubMethod.h"
#ifndef SENTRY_REDISSUBSERVICE_H
#define SENTRY_REDISSUBSERVICE_H

namespace Sentry
{
    class SubService : public Component
    {
    public:
        SubService() = default;
        virtual ~SubService() = default;
    public:
        void GetSubMethods(std::vector<std::string> &methods);
        bool Publish(const std::string & func, const std::string & message);
        virtual const std::string & GetName() { return this->GetTypeName(); }
    protected:
        template<typename T>
        bool Bind(std::string name, JsonSubFunction<T> func)
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
        bool Bind(std::string name, ProtoSubFuncrion<T,T1> func)
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
        std::unordered_map<std::string, SubMethod *> mSubMethodMap;
    };
#define BIND_SUB_FUNCTION(func) LOG_CHECK_RET_FALSE(this->Bind(GetFunctionName(#func), &func))
}

#endif //SENTRY_REDISSUBSERVICE_H
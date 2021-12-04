//
// Created by zmhy0073 on 2021/11/26.
//

#ifndef GAMEKEEPER_JSONSERVICECOMPONENT_H
#define GAMEKEEPER_JSONSERVICECOMPONENT_H
#include"ServiceComponent.h"
#include"Method/JsonServiceMethod.h"
namespace GameKeeper
{
    class JsonServiceComponent : public ServiceComponent
    {
    public:
        JsonServiceComponent() = default;
        ~JsonServiceComponent() override = default;

    public:
        const std::string & GetServiceName() final { return this->GetTypeName();}
    protected:
        template<typename T>
        bool Bind(std::string name, JsonServiceMethodType1<T> func) {
            return this->AddMethod(new JsonServiceMethod1<T>(name, (T*)this, func));
        }
        template<typename T>
        bool Bind(std::string name, JsonServiceMethodType11<T> func) {
            return this->AddMethod(new JsonServiceMethod1<T>(name, (T*)this, func));
        }
        template<typename T>
        bool Bind(std::string name, JsonServiceMethodType2<T> func) {
            return this->AddMethod(new JsonServiceMethod2<T>(name, (T*)this, func));
        }
        template<typename T>
        bool Bind(std::string name, JsonServiceMethodType22<T> func) {
            return this->AddMethod(new JsonServiceMethod2<T>(name, (T*)this, func));
        }
        template<typename T>
        bool Bind(std::string name, JsonServiceMethodType3<T> func) {
            return this->AddMethod(new JsonServiceMethod3<T>(name, (T*)this, func));
        }
        template<typename T>
        bool Bind(std::string name, JsonServiceMethodType33<T> func) {
            return this->AddMethod(new JsonServiceMethod3<T>(name, (T*)this, func));
        }
    };
#define BindJsoMethod(func) LOG_CHECK_RET_FALSE(this->Bind(GetFunctionName(#func), &func))
}
#endif //GAMEKEEPER_JSONSERVICECOMPONENT_H

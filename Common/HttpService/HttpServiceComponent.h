//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICECOMPONENT_H
#define GAMEKEEPER_HTTPSERVICECOMPONENT_H
#include <Component/Component.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    class HttpServiceComponent : public Component
    {
    public:
        HttpServiceComponent() = default;
        virtual ~HttpServiceComponent() = default;
    public:
        HttpServiceMethod * GetMethod(const std::string & path);
    protected:
        bool Awake() override;

        template<typename T>
        bool Add(const std::string & path, HttpServiceMethodType<T> func, T * o)
        {
            auto iter = this->mMethodMap.find(path);
            if (iter != this->mMethodMap.end())
            {
                return false;
            }
            this->mMethodMap.emplace(path, new HttpServiceMethod1<T>(o, func));
            return true;
        }

    private:
        std::unordered_map<std::string, HttpServiceMethod *> mMethodMap;
    };
}
#endif //GAMEKEEPER_HTTPSERVICECOMPONENT_H

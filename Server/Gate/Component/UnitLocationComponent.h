//
// Created by zmhy0073 on 2022/10/11.
//

#ifndef APP_UNITLOCATIONCOMPONENT_H
#define APP_UNITLOCATIONCOMPONENT_H
#include"Component/Component.h"
namespace Sentry
{
    class UnitLocationComponent : public Component
    {
    public:
        UnitLocationComponent() = default;
        ~UnitLocationComponent() = default;

    private:
        bool LateAwake() final;
    public:
        bool Del(const std::string & name);
        bool Add(const std::string & addres);
        bool Add(const std::string & name, const std::string & address);
        bool Get(const std::string & name, std::string & address);
        const std::string & GetAddress() const { return this->mAddress;}
    private:
        std::string mAddress;
        class OuterNetComponent * mOuterComponent;
        std::unordered_map<std::string, std::string> mLocationMap;
    };
}


#endif //APP_UNITLOCATIONCOMPONENT_H

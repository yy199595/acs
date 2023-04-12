//
// Created by zmhy0073 on 2022/11/2.
//

#ifndef APP_REDISSTRINGCOMPONENT_H
#define APP_REDISSTRINGCOMPONENT_H
#include"Entity/Component/Component.h"

namespace Tendo
{
    class RedisStringComponent : public Component
    {
    public:
        RedisStringComponent() = default;
    public:
        long long AddCounter(const std::string & key);
        long long SubCounter(const std::string & key);

    public:
        bool Set(const std::string & key, const std::string & value);
        std::unique_ptr<std::string> Get(const std::string & key);
        std::unique_ptr<std::string> Append(const std::string & key, const std::string & value);
    protected:
        bool LateAwake() final;
    private:
        class RedisComponent * mComponent;
    };
}


#endif //APP_REDISSTRINGCOMPONENT_H

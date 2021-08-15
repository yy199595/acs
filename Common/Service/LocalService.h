#pragma once

#include "ServiceBase.h"
#include <NetWork/NetWorkAction.h>

namespace Sentry
{
    class LocalService : public ServiceBase
    {
    public:
        LocalService();

        virtual ~LocalService() {}

    public:
        bool HasMethod(const std::string &action) final;

    public:
        bool OnInit() override;

    public:
        void GetServiceList(std::vector<shared_ptr<LocalActionProxy>> &service) final;

    private:
        virtual bool InvokeMethod(NetMessageProxy *) final;

        virtual bool InvokeMethod(const std::string &address, NetMessageProxy *) final;

    protected:
        bool BindFunction(std::string name, LocalAction1 action);

        template<typename T1>
        bool BindFunction(std::string name, LocalAction2<T1> action);

        template<typename T1>
        bool BindFunction(std::string name, LocalAction4<T1> action);

        template<typename T1, typename T2>
        bool BindFunction(std::string name, LocalAction3<T1, T2> action);

    protected:
        bool BindFunction(const std::string &name, shared_ptr<LocalActionProxy> actionBox);

    private:
        class CoroutineManager *mCorManager;
        std::unordered_map<std::string, shared_ptr<LocalActionProxy>> mActionMap;
    };

    template<typename T1>
    inline bool LocalService::BindFunction(std::string name, LocalAction2<T1> action)
    {
        typedef LocalActionProxy2<T1> ActionProxyType;
        return this->BindFunction(name, make_shared<ActionProxyType>(action));
    }

    template<typename T1>
    inline bool LocalService::BindFunction(std::string name, LocalAction4<T1> action)
    {
        typedef LocalActionProxy4<T1> ActionProxyType;
        return this->BindFunction(name, make_shared<ActionProxyType>(action));
    }

    template<typename T1, typename T2>
    inline bool LocalService::BindFunction(std::string name, LocalAction3<T1, T2> action)
    {
        return this->BindFunction(name, make_shared<LocalActionProxy3<T1, T2>>(action));
    }

#define REGISTER_FUNCTION_0(func) this->BindFunction(GetFunctionName(#func), std::bind(&func, this, args1))
#define REGISTER_FUNCTION_1(func, t1) this->BindFunction<t1>(GetFunctionName(#func), std::bind(&func, this, args1, args2))
#define REGISTER_FUNCTION_2(func, t1, t2) this->BindFunction<t1, t2>(GetFunctionName(#func), std::bind(&func, this, args1, args2, args3))

}// namespace Sentry
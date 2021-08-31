#pragma once

#include "ServiceBase.h"
#include <NetWork/NetWorkAction.h>

namespace Sentry
{

	inline std::string GetFunctionName(const std::string func)
	{
		size_t pos = func.find("::");
		return func.substr(pos + 2);
	}

    class LocalService : public ServiceBase
    {
    public:
        LocalService();

        virtual ~LocalService() {}

    public:
        bool HasMethod(const std::string &action) final;
		void GetMethods(std::vector<LocalActionProxy*> & methods) final;
		const std::string &GetServiceName()final { return this->GetTypeName(); }
    public:
        bool Awake() override;

    private:
        virtual XCode InvokeMethod(PacketMapper *) final;
     
    protected:
        bool BindFunction(std::string name, LocalAction1 action);

        template<typename T1>
        bool BindFunction(std::string name, LocalAction2<T1> action);

        template<typename T1>
        bool BindFunction(std::string name, LocalAction4<T1> action);

        template<typename T1, typename T2>
        bool BindFunction(std::string name, LocalAction3<T1, T2> action);

	protected:
		template<typename O>
		bool BindFunction(std::string name, XCode (O::* func));

    protected:
        bool BindFunction(LocalActionProxy * actionBox);

    private:
        class CoroutineComponent *mCorComponent;
        std::unordered_map<std::string, LocalActionProxy *> mActionMap;
    };

    template<typename T1>
    inline bool LocalService::BindFunction(std::string name, LocalAction2<T1> action)
    {
        typedef LocalActionProxy2<T1> ActionProxyType;
        return this->BindFunction(new ActionProxyType(name, action));
    }

    template<typename T1>
    inline bool LocalService::BindFunction(std::string name, LocalAction4<T1> action)
    {
        typedef LocalActionProxy4<T1> ActionProxyType;
        return this->BindFunction(new ActionProxyType(name, action));
    }

    template<typename T1, typename T2>
    inline bool LocalService::BindFunction(std::string name, LocalAction3<T1, T2> action)
    {
        return this->BindFunction(new LocalActionProxy3<T1, T2>(name, action));
    }

#define REGISTER_FUNCTION_0(func) this->BindFunction(GetFunctionName(#func), std::bind(&func, this, args1))
#define REGISTER_FUNCTION_1(func, t1) this->BindFunction<t1>(GetFunctionName(#func), std::bind(&func, this, args1, args2))
#define REGISTER_FUNCTION_2(func, t1, t2) this->BindFunction<t1, t2>(GetFunctionName(#func), std::bind(&func, this, args1, args2, args3))

}// namespace Sentry
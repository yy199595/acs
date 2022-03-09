//
// Created by zmhy0073 on 2021/11/19.
//
#include"OperatorComponent.h"
#include"Timer/TimerComponent.h"
#include"Object/App.h"
namespace Sentry
{
    bool OperatorComponent::Awake()
    {
        this->mTimerComponent = nullptr;
        return true;
    }

    bool OperatorComponent::LateAwake()
    {
        this->mTimerComponent = this->GetComponent<TimerComponent>();
        std::vector<Component *> components;
        this->GetComponents(components);
        for (Component *component: components)
        {
            this->AddRefreshTimer(component);
        }
        return true;
    }

    void OperatorComponent::StartHotfix()
    {
        std::vector<Component *> components;
        this->GetComponents(components);
        for(Component * component : components)
        {
            if(auto hotfix = dynamic_cast<IHotfix*>(component))
            {
                hotfix->OnHotFix();
                LOG_INFO(component->GetName()," start hotfix");
            }
        }
    }

    bool OperatorComponent::StartLoadConfig()
    {
        std::vector<Component *> components;
        this->GetComponents(components);
        for(Component * component : components)
        {
            if(auto loadConfig = dynamic_cast<ILoadConfig*>(component))
            {
                if(!loadConfig->OnLoadConfig())
                {
                    LOG_ERROR(component->GetName(), " load config error");
                    return false;
                }
                LOG_INFO("{0} start hotfix", component->GetName());
            }
        }
        return true;
    }

    void OperatorComponent::AddRefreshTimer(Component * component)
    {
        auto zeroRefresh = dynamic_cast<IZeroRefresh *>(component);
        if (zeroRefresh == nullptr)
        {
            return;
        }
        int hour, minute, second = 0;
        zeroRefresh->GetRefreshTime(hour, minute);
        long long nextTime = Helper::Time::GetNewTime(0, hour, minute);
        if(Helper::Time::GetSecTimeStamp() - nextTime > 0)
        {
            nextTime = Helper::Time::GetNewTime(1, hour, minute);
        }
        long long ms = nextTime - Helper::Time::GetSecTimeStamp();
        this->mTimerComponent->AsyncWait(ms * 1000, &OperatorComponent::StartRefreshDay,
                                         this, component->GetName());
#ifdef __DEBUG__
        Helper::Time::GetHourMinSecond(ms, hour, minute, second);
		std::string str = fmt::format("refresh {0} new day after "
									  "{1}小时{1}分{2}分", component->GetName(), hour, minute, second);
		LOG_DEBUG(str);
#endif
    }

    void OperatorComponent::StartRefreshDay(const std::string & name)
    {
         auto component = this->GetComponent<Component>(name);
        if(auto zeroRefresh = dynamic_cast<IZeroRefresh*>(component))
        {
            zeroRefresh->OnZeroRefresh();
            this->AddRefreshTimer(component);
        }
    }
}
//
// Created by zmhy0073 on 2021/11/19.
//
#include"OperatorComponent.h"
#include"Timer/TimerComponent.h"
#include"Core/App.h"
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
                LOG_INFO(component->GetTypeName()," start hotfix");
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
                    LOG_ERROR(component->GetTypeName(), " load config error");
                    return false;
                }
                LOG_INFO("{0} start hotfix", component->GetTypeName());
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

        time_t t = time(nullptr);
        struct tm *pt = localtime(&t);
        struct tm nextRefresh = *pt;
        zeroRefresh->GetRefreshTime(nextRefresh.tm_hour, nextRefresh.tm_min);
        if (pt->tm_hour > nextRefresh.tm_hour)
        {
            nextRefresh.tm_mday += 1;
        }
        else if (pt->tm_hour == nextRefresh.tm_hour && pt->tm_min > nextRefresh.tm_min)
        {
            nextRefresh.tm_mday += 1;
        }
        nextRefresh.tm_sec = 0;
        time_t nextTime = mktime(&nextRefresh) - time(nullptr);
        this->mTimerComponent->AsyncWait(nextTime * 1000, &OperatorComponent::StartRefreshDay, this,
                                         component->GetTypeName());
#ifdef __DEBUG__
        int hour, min, second = 0;
        Helper::Time::GetHourMinSecond(nextTime, hour, min, second);
        LOG_DEBUG("Refresh the new day after", hour, 'h', min, 'm', second, 's');
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
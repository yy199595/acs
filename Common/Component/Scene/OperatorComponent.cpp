//
// Created by zmhy0073 on 2021/11/19.
//
#include"OperatorComponent.h"
#include"Timer/TimerComponent.h"
namespace GameKeeper
{
    bool OperatorComponent::Awake()
    {
        long long nowTime = TimeHelper::GetSecTimeStamp();
        long long netxTime = TimeHelper::GetToDayZeroTime();
        long long ms = (netxTime - nowTime) * 1000;
        GKAssertRetFalse_F( this->mTimerComponent = this->GetComponent<TimerComponent>());
        this->mRefreshTimerId = this->mTimerComponent->AddTimer(ms, &OperatorComponent::StartRefreshDay, this);
#ifdef __DEBUG__
        GKDebugLog("Refresh the new day after " << ms / 1000 << "s");
#endif
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
                GKDebugInfo(component->GetTypeName() << " start hotfix");
            }
        }
    }

    bool OperatorComponent::StartLoadConnfig()
    {
        std::vector<Component *> components;
        this->GetComponents(components);
        for(Component * component : components)
        {
            if(auto loadCOnfig = dynamic_cast<ILoadConfig*>(component))
            {
                if(!loadCOnfig->OnLoadConfig())
                {
                    GKDebugError(component->GetTypeName() << " load config error");
                    return false;
                }
                GKDebugInfo(component->GetTypeName() << " start hotfix");
            }
        }
        return true;
    }

    void OperatorComponent::StartRefreshDay()
    {

    }
}
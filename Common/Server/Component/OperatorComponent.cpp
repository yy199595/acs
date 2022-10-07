//
// Created by zmhy0073 on 2021/11/19.
//
#include"OperatorComponent.h"
#include"Component/TimerComponent.h"
#include"App/App.h"
namespace Sentry
{
	bool OperatorComponent::LateAwake()
	{
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			Component * component = this->GetComponent<Component>(name);
			this->AddRefreshTimer(component);
		}
		return true;
	}

	void OperatorComponent::StartHotfix()
	{
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			IHotfix* hotfixComponent = this->GetComponent<IHotfix>(name);
			if(hotfixComponent != nullptr)
			{
				hotfixComponent->OnHotFix();
			}
		}
	}

	bool OperatorComponent::StartLoadConfig()
	{
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			ILoadConfig* loadComponent = this->GetComponent<ILoadConfig>(name);
			if(loadComponent != nullptr)
			{
				if(!loadComponent->OnLoadConfig())
				{
					LOG_ERROR(name << " load config error");
					return false;
				}
			}
			LOG_INFO(name << " start hotfix");
		}
		return true;
	}

	void OperatorComponent::AddRefreshTimer(Component* component)
	{
		auto zeroRefresh = dynamic_cast<IZeroRefresh*>(component);
		if (zeroRefresh == nullptr)
		{
			return;
		}
		int hour, minute = 0;
		zeroRefresh->GetRefreshTime(hour, minute);
		long long nextTime = Helper::Time::GetNewTime(0, hour, minute);
		if (Helper::Time::GetNowSecTime() - nextTime > 0)
		{
			nextTime = Helper::Time::GetNewTime(1, hour, minute);
		}
		long long ms = nextTime - Helper::Time::GetNowSecTime();
		this->mTimerComponent->DelayCall(ms * 1000, &OperatorComponent::StartRefreshDay,
				this, component->GetName());
	}

	void OperatorComponent::StartRefreshDay(const std::string& name)
	{
		Component * component = this->GetComponent<Component>(name);
		if (IZeroRefresh * zeroRefresh = dynamic_cast<IZeroRefresh*>(component))
		{
			zeroRefresh->OnZeroRefresh();
			this->AddRefreshTimer(component);
		}
	}
}
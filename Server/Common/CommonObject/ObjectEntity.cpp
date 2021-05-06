#include"ObjectEntity.h"
#include<CommonDefine/CommonDef.h>
#include<CommonCore/Applocation.h>
#include<CommonModule/Component.h>
#include<CommonModule/SayNoFactory.h>
#include<CommonUtil/ReflectionHelper.h>


namespace SoEasy
{
	ObjectEntity::ObjectEntity(const long long id)
		: mGameObjectId(id)
	{

	}

	Component * ObjectEntity::AddComponentByName(const std::string name)
	{
		Component * component = this->GetComponentByName(name);
		if (component == nullptr)
		{			
			Object * obj = ObjectFactory::Get()->CreateObjectByName(name);
			if (obj != nullptr)
			{
				component = dynamic_cast<Component*>(obj);
				SayNoAssertRetNull_F(component);
				this->mWaitStartComponents.push(component);
				this->mComponentMap.insert(std::make_pair(name, component));
			}
		}
		return component;
	}

	void ObjectEntity::GetAllComponent(SayNoArray<Component*>& mConponentArray)
	{
		auto iter = this->mComponentMap.begin();
		for (; iter != this->mComponentMap.end(); iter++)
		{
			Component * component = iter->second;
			mConponentArray.push_back(component);
		}
	}

	void ObjectEntity::OnDestory()
	{
		this->DestoryComponents();
	}

	Component * ObjectEntity::GetComponentByName(const std::string name)
	{
		ComponentIter iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			Component * component = iter->second;
			return component->IsActive() ? component : nullptr;
		}
		return nullptr;
	}

	bool ObjectEntity::RemoveComponentByName(const std::string & name)
	{
		ComponentIter iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			Component * component = iter->second;
			component->SetActive(false);
			return true;
		}
		return false;
	}

	void ObjectEntity::PollComponent(float t)
	{
		while (!this->mWaitStartComponents.empty())
		{
			Component * component = this->mWaitStartComponents.front();
			component->OnFrameStart();
			this->mWaitStartComponents.pop();
		}

		ComponentIter iterator = this->mComponentMap.begin();
		for (; iterator != this->mComponentMap.end();)
		{
			Component * pComponent = iterator->second;
			if (pComponent != nullptr)
			{
				if (pComponent->IsActive())
				{
					this->mComponentMap.erase(iterator++);
					pComponent->Destory();
					continue;
				}
				pComponent->OnFrameUpdate(t);		
			}
			iterator++;
		}
	}

	

	void ObjectEntity::DestoryComponents()
	{
		ComponentIter iterator = this->mComponentMap.begin();
		for (; iterator != this->mComponentMap.end(); iterator++)
		{
			Component * component = iterator->second;
			component->Destory();
		}
		this->mComponentMap.clear();
		while (!this->mWaitStartComponents.empty())
		{
			Component * component = this->mWaitStartComponents.front();
			component->Destory();
			this->mWaitStartComponents.pop();
		}
	}
}






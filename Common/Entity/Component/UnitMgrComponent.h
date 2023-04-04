#pragma once
#include<vector>
#include<unordered_map>
#include"Core/Component/Component.h"

namespace Tendo
{
	class Unit;
	class UnitMgrComponent : public Component
	{
	 public:
		UnitMgrComponent() = default;
		~UnitMgrComponent() = default;
	 public:
		bool LateAwake() final;
	 public:
		template<typename T>
		T * Find(long long userId);
		Unit * Find(long long userId);
	 public:
		bool Del(long long unitId);
		bool Add(std::unique_ptr<Unit> gameObject);
		void GetUnits(std::vector<Unit *>& gameObjects);
		size_t GetUnitCount() const { return this->mGameObjects.size(); }
	 private:
		void StartComponents(long long objectId);
	 private:
		class AsyncMgrComponent* mCorComponent;
		std::unordered_map<long long, std::unique_ptr<Unit>> mGameObjects;
	};
	template<typename T>
	T* UnitMgrComponent::Find(long long userId)
	{
		auto iter = this->mGameObjects.find(userId);
		if (iter == this->mGameObjects.end())
		{
			return nullptr;
		}
		return static_cast<T*>(iter->second.get());
	}
}
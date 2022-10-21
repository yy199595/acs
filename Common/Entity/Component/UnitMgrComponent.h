#pragma once

#include"Component/Component.h"

namespace Sentry
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

	 public:
		bool Del(long long unitId);
		Unit * Find(long long userId);
		bool Add(std::unique_ptr<Unit> gameObject);
		void GetUnits(std::vector<Unit *>& gameObjects);
		size_t GetUnitCount() const { return this->mGameObjects.size(); }
	 private:
		void StartComponents(long long objectId);
	 private:
		class TaskComponent* mCorComponent;
		std::unordered_map<long long, std::unique_ptr<Unit>> mGameObjects;
	};
}
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
		std::shared_ptr<Unit> Find(long long userId);
	 public:
		bool Del(long long gameObjectId);
		bool Add(std::shared_ptr<Unit> gameObject);
		bool Del(std::shared_ptr<Unit> gameObject);
		size_t GetEntityCount()
		{
			return this->mGameObjects.size();
		}
		void GetGameObjects(std::vector<std::shared_ptr<Unit>>& gameObjects);
	 private:
		void StartComponents(long long objectId);
	 private:
		class TaskComponent* mCorComponent;
		std::unordered_map<long long, std::shared_ptr<Unit>> mGameObjects;
		std::unordered_map<std::string, std::shared_ptr<Unit>> mAddressGameObjects;
	};
}
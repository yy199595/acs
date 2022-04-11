#pragma once

#include "Component/Component.h"

namespace Sentry
{
	class Entity;
	class EntityMgrComponent : public Component
	{
	 public:
		EntityMgrComponent() = default;
		~EntityMgrComponent() final = default;
	 public:
		bool LateAwake() final;
	 public:
		std::shared_ptr<Entity> Find(long long userId);
	 public:
		bool Del(long long gameObjectId);
		bool Add(std::shared_ptr<Entity> gameObject);
		bool Del(std::shared_ptr<Entity> gameObject);
		size_t GetEntityCount()
		{
			return this->mGameObjects.size();
		}
		void GetGameObjects(std::vector<std::shared_ptr<Entity>>& gameObjects);
	 private:
		void StartComponents(long long objectId);
	 private:
		class TaskComponent* mCorComponent;
		std::unordered_map<long long, std::shared_ptr<Entity>> mGameObjects;
		std::unordered_map<std::string, std::shared_ptr<Entity>> mAddressGameObjects;
	};
}
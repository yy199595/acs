#pragma once

#include "Component.h"

namespace Sentry
{
	class Entity;
    class  GameObjectComponent : public Component
	{
	public:
		GameObjectComponent() = default;
		~GameObjectComponent() final = default;
	public:
		bool Awake() final;
        bool LateAwake() final;
	public:
		Entity * Find(long long id);
        Entity * Find(const std::string & address);
		bool Add(Entity * gameObject);

		bool Del(long long gameObjectId);
		bool Del(Entity * gameObject);
		
		void GetGameObjects(std::vector<Entity *> & gameObjects);
	private:
		void StartComponents(long long objectId);
	private:
		class TaskComponent * mCorComponent;
		std::unordered_map<long long, Entity *> mGameObjects;
        std::unordered_map<std::string, Entity *> mAddressGameObjects;
	};
}
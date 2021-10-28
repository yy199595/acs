#pragma once

#include "Component.h"

namespace Sentry
{
	class GameObject;
	class GameObjectComponent : public Component
	{
	public:
		GameObjectComponent() = default;
		~GameObjectComponent() = default;
	public:
		bool Awake() { return true; }
	public:
		GameObject * Find(long long id);
        GameObject * Find(const std::string & address);
		bool Add(GameObject * gameObject);

		bool Del(long long gameObjectId);
		bool Del(GameObject * gameObject);
		
		void GetGameObjects(std::vector<GameObject *> & gameObjects);
	private:
		void StartComponents(std::vector<Component *>  & components);
	private:
		class CoroutineComponent * mCorComponent;
		std::unordered_map<long long, GameObject *> mGameObjects;
        std::unordered_map<std::string, GameObject *> mAddressGameObjects;
	};
}
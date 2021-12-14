#pragma once

#include "Component.h"

namespace GameKeeper
{
	class GameObject;
    class  GameObjectComponent : public Component
	{
	public:
		GameObjectComponent() = default;
		~GameObjectComponent() final = default;
	public:
		bool Awake() final;
        bool LateAwake() final;
	public:
		GameObject * Find(long long id);
        GameObject * Find(const std::string & address);
		bool Add(GameObject * gameObject);

		bool Del(long long gameObjectId);
		bool Del(GameObject * gameObject);
		
		void GetGameObjects(std::vector<GameObject *> & gameObjects);
	private:
		void StartComponents(long long objectId);
	private:
		class TaskComponent * mCorComponent;
		std::unordered_map<long long, GameObject *> mGameObjects;
        std::unordered_map<std::string, GameObject *> mAddressGameObjects;
	};
}
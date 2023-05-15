#pragma once
#include<vector>
#include<unordered_map>
#include"Component.h"

namespace Tendo
{
	class Player;
	class PlayerMgrComponent : public Component
	{
	 public:
		PlayerMgrComponent() = default;
		~PlayerMgrComponent() = default;
	 public:
		bool LateAwake() final;
	 public:
		Player * GetPlayer(long long userId);
	 public:
		bool DelPlayer(long long userId);
		bool Add(std::unique_ptr<Player> gameObject);
		void GetPlayers(std::vector<Player *>& players);
		size_t GetPlayerCount() const { return this->mPlayers.size(); }
	 private:
		void StartComponents(long long objectId);
	 private:
		class CoroutineComponent* mCorComponent;
		std::unordered_map<long long, std::unique_ptr<Player>> mPlayers;
	};
}
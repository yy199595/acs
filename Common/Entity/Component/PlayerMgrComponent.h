#pragma once
#include<vector>
#include<memory>
#include<unordered_map>
#include"Component.h"
#include"Entity/Unit/Player.h"
namespace Tendo
{
	//class Player;
	class PlayerMgrComponent final : public Component
	{
	 public:
		PlayerMgrComponent()  = default;
		~PlayerMgrComponent() = default;
	 public:
		bool LateAwake() final;
	 public:
		Player * GetPlayer(long long playerId);
		bool AddPlayer(std::unique_ptr<Player> player);
	 public:
		bool DelPlayer(long long userId);
		void GetPlayers(std::vector<Player *>& players);
		inline size_t GetPlayerCount() const { return this->mPlayers.size(); }
	 private:
		void StartComponents(long long objectId);
	private:
		std::unordered_map<long long, std::unique_ptr<Player>> mPlayers;
	};
}
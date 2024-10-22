//
// Created by 64658 on 2024/10/22.
//

#include "Room.h"
#include "Entity/Actor/App.h"
#include "Entity/Component/ActorComponent.h"
namespace acs
{
	Room::Room(long long id)
		: mRoomId(id)
	{
		this->mActor = acs::App::ActorMgr();
	}

	bool Room::Add(long long playerId)
	{
		auto iter = this->mPlayers.find(playerId);
		if(iter != this->mPlayers.end())
		{
			return false;
		}
		this->mPlayers.insert(playerId);
		return true;
	}

	bool Room::Del(long long playerId)
	{
		auto iter = this->mPlayers.find(playerId);
		if(iter == this->mPlayers.end())
		{
			return false;
		}
		this->mPlayers.erase(iter);
		return true;
	}

	int Room::Send(const std::string& func, const pb::Message& message)
	{
		int count = 0;
		for(const long long playerId : this->mPlayers)
		{
			Player * player = this->mActor->GetPlayer(playerId);
			if(player != nullptr)
			{
				count++;
				player->Send(func, message);
			}
		}
		return count;
	}
}
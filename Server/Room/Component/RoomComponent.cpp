//
// Created by 64658 on 2024/10/22.
//

#include "RoomComponent.h"

namespace acs
{
	RoomComponent::RoomComponent()
	{

	}

	bool RoomComponent::AddPlayer(long long roomId, long long playerId)
	{
		auto iter = this->mRooms.find(roomId);
		if(iter == this->mRooms.end())
		{
			std::unique_ptr<Room> room = std::make_unique<Room>(roomId);
			{
				room->Add(playerId);
			}
			this->mRooms.emplace(roomId, std::move(room));
			return true;
		}
		return iter->second->Add(playerId);
	}

	bool RoomComponent::DelPlayer(long long roomId, long long playerId)
	{
		auto iter = this->mRooms.find(roomId);
		if(iter == this->mRooms.end())
		{
			return false;
		}
		if(!iter->second->Del(playerId))
		{
			return false;
		}
		if(iter->second->Count() <= 0)
		{
			this->mRooms.erase(iter);
		}
		return true;
	}
}
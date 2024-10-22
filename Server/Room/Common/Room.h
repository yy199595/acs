//
// Created by 64658 on 2024/10/22.
//

#ifndef APP_ROOM_H
#define APP_ROOM_H
#include <unordered_set>
#include "Proto/Include/Message.h"
#include "Entity/Component/ActorComponent.h"
namespace acs
{
	class Room
	{
	public:
		explicit Room(long long id);
	public:
		bool Add(long long playerId);
		bool Del(long long playerId);
	public:
		long long RoomID() const { return this->mRoomId; }
		size_t Count() const { return this->mPlayers.size(); }
		int Send(const std::string & func, const pb::Message & message);
	private:
		long long mRoomId;
		ActorComponent * mActor;
		std::unordered_set<long long> mPlayers;
	};
}


#endif //APP_ROOM_H

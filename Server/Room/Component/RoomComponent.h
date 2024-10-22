//
// Created by 64658 on 2024/10/22.
//

#ifndef APP_ROOMCOMPONENT_H
#define APP_ROOMCOMPONENT_H
#include <unordered_map>
#include <unordered_set>
#include "Room/Common/Room.h"
#include "Entity/Component/Component.h"
namespace acs
{
	class RoomComponent : public Component
	{
	public:
		RoomComponent();
	public:
		bool DelPlayer(long long roomId, long long playerId);
		bool AddPlayer(long long roomId, long long playerId);
	private:
		std::unordered_map<long long, std::unique_ptr<Room>> mRooms;
	};
}


#endif //APP_ROOMCOMPONENT_H

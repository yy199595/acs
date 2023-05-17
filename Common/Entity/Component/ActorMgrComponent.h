//
// Created by yjz on 2023/5/17.
//

#ifndef _ACTORMGRCOMPONENT_H_
#define _ACTORMGRCOMPONENT_H_
#include"Component.h"
#include"Entity/Actor/Actor.h"
namespace Tendo
{
	class ActorMgrComponent : public Component
	{
	 public:
		bool AddActor(Actor * actor);
		Actor * GetActor(long long id);
		Actor * GetActor(const std::string & addr);
		Actor * RandomActor(const std::string & name);
		Actor * GetOrCreateActor(long long id, const std::string & addr);
	 public:
		bool DelActor(long long id);
		bool DelActor(const std::string & addr);
	 private:
		bool LateAwake() final;
	 private:
		std::unordered_map<long long, Actor*> mActors;
		std::unordered_map<std::string, Actor*> mAddrActors;
		std::unordered_map<std::string, std::vector<long long>> mActorNames;
	};
}

#endif //_ACTORMGRCOMPONENT_H_

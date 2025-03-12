//
// Created by yjz on 2023/5/17.
//

#ifndef APP_ACTORCOMPONENT_H
#define APP_ACTORCOMPONENT_H
#include "Entity/Actor/Server.h"
#include "Core/Map/HashMap.h"

namespace Lua
{
	class LuaModule;
};

namespace actor
{
	class Group
	{
	public:
		Group() : mIndex(0) { }
	public:
		bool AddItem(long long id);
		bool Random(long long & id);
		bool Hash(long long hash, long long & id);
		const std::vector<long long> & GetItems() const { return this->mItems; }
	private:
		size_t mIndex;
		std::vector<long long> mItems;
	};
}

namespace acs
{
	class ActorComponent final : public Component, public IServerRecord
	{
	public:
		ActorComponent();
	public:
		bool DelActor(long long id);
		Actor * GetActor(long long id);
		template<typename T>
		inline T * GetActor(long long id) { return (T*) this->GetActor(id); }
		bool AddActor(std::unique_ptr<Actor> actor, bool group = false);
	public:
		actor::Group * GetGroup(const std::string & name);
		bool AddGroup(const std::string & name, long long id);
		inline size_t GetActorCount() const { return this->mActors.size(); }
		bool GetListen(int id, const std::string & net, std::string & address);
	private:
		bool LateAwake() final;
		bool LoadServerFromFile();
		void OnRecord(json::w::Document &document) final;
	private:
		std::unordered_map<std::string, actor::Group> mGroups;
		std::unordered_map<long long, std::unique_ptr<Actor>> mActors;
		std::unordered_map<std::string, std::vector<long long>> mActorNames;
	};
}

#endif //APP_ACTORCOMPONENT_H

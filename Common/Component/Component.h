#pragma once

#include"IComponent.h"

#include <XCode/XCode.h>
#include<Object/Object.h>
#include <Protocol/db.pb.h>
#include <Protocol/c2s.pb.h>
#include <Protocol/com.pb.h>
#include <Protocol/s2s.pb.h>
#include <Define/CommonDef.h>
#include <Method/MethodProxy.h>

namespace Sentry
{
	class Component;
	class Type
	{
	public:
		Type(size_t hash, std::string name) :
			Hash(hash), Name(name) { }
	public:
		virtual Component * New() = 0;
	public:
		const size_t Hash;
		const std::string Name;
	};
}

namespace Sentry
{
	class GameObject;
	class Component : public Object
	{
	public:
		Component();

		virtual ~Component() {}

	public:
		friend class GameObject;
		friend class ComponentHelper;
		inline long long GetGameObjectID()
		{
			return gameObjectID;
		}

		inline GameObject * GetObject()
		{
			return this->gameObject;
		}
		inline Type * GetType() { return this->mType; }

	public:
		bool IsComponent() override
		{
			return true;
		}
	public:
		virtual bool Awake() = 0;

		virtual void Start() { };

		virtual int GetPriority() { return 100; }
	protected:
		Type * mType;
		long long gameObjectID;
		GameObject * gameObject;
		std::unordered_map<std::string, void*> mEventMap;
	};
}
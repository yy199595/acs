#pragma once

#include"IComponent.h"
#include <XCode/XCode.h>
#include<Object/Object.h>
#include <Protocol/db.pb.h>
#include <Protocol/c2s.pb.h>
#include <Protocol/com.pb.h>
#include <Protocol/s2s.pb.h>
#include <Define/CommonDef.h>

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

		inline long long GetGameObjectID()
		{
			return gameObjectID;
		}

		inline GameObject * GetObject()
		{
			return this->gameObject;
		}

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
		long long gameObjectID;
		GameObject * gameObject;
	};
}
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
#include <Define/CommonTypeDef.h>

namespace GameKeeper
{
	class Component;
	class Type
	{
	public:
		Type(size_t hash, std::string name) :
			Hash(hash), Name(std::move(name)) { }
	public:
		virtual Component * New() = 0;
	public:
		const size_t Hash;
		const std::string Name;
	};
}

namespace GameKeeper
{
	class GameObject;
	class Component : public Object
	{
	public:
		Component();
		~Component() override = default;

	public:
		friend class GameObject;
		friend class ComponentHelper;
		inline long long GetGameObjectID() const
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
		template<typename T>
		T * GetComponent();

        template<typename T>
        T * GetComponent(const std::string & name);

        void GetComponents(std::vector<Component *> & components);

        Component * GetByName(const std::string & name);
	private:
		Component * GetByHash(size_t hash);
	protected:
		Type * mType;
		long long gameObjectID;
		GameObject * gameObject;
		std::unordered_map<std::string, void*> mEventMap;
	};
	template<typename T>
	inline T * Component::GetComponent()
	{
		const size_t hash = typeid(T).hash_code();
		return static_cast<T*>(this->GetByHash(hash));
	}

    template<typename T>
    inline T * Component::GetComponent(const std::string & name)
    {
        Component *component = this->GetByName(name);
        if (component == nullptr)
        {
            return nullptr;
        }
        return static_cast<T *>(component);
    }

}
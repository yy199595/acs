#pragma once

#include"IComponent.h"

#include <XCode/XCode.h>
#include<Object/Object.h>
#include <Protocol/db.pb.h>
#include <Protocol/c2s.pb.h>
#include <Protocol/com.pb.h>
#include <Protocol/s2s.pb.h>
#include <Define/CommonLogDef.h>
#include <Method/MethodProxy.h>
#include <Define/CommonTypeDef.h>
using namespace google::protobuf;
namespace Sentry
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

namespace Sentry
{
	class Entity;
	class Component : public Object
	{
	public:
		Component();
		~Component() override = default;

	public:
		friend class Entity;
		friend class ComponentFactory;
		inline long long GetEntityId() const
		{
			return this->mEntityId;
		}

		inline Entity * GetEntity()
		{
			return this->mEntity;
		}
		inline Type * GetType() { return this->mType; }

	public:
		bool IsComponent() override
		{
			return true;
		}
	public:

		virtual bool Awake() = 0; //组件创建的时候调用
        
        virtual bool LateAwake() = 0; // 所有组件加载完成之后调用

        virtual void OnComplete() { } //在所有组件的start load 都完成之后调用
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
		Entity * mEntity;
        long long mEntityId;
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
        return dynamic_cast<T *>(component);
    }

    inline std::string GetFunctionName(const std::string func)
    {
        size_t pos = func.find("::");
        return func.substr(pos + 2);
    }


}
#pragma once

#include"Object.h"

namespace Sentry
{
    class Component;

    class GameObject : public Object, public std::enable_shared_from_this<GameObject>
    {
    public:
        GameObject(const long long id);

        GameObject(const long long id, const std::string &address);

        virtual ~GameObject() {};
    public:
        template<typename T>
        inline T *AddComponent();

        template<typename T>
        inline T *GetComponent();

        template<typename T>
        inline bool RemoveComponent();

        bool RemoveComponent(const std::string &name);

        template<typename T>
        inline T *GetOrAddComponent();

    public:
        void DestoryComponents();

        Component *AddComponentByName(const std::string name);

        Component *GetComponentByName(const std::string name);

        bool RemoveComponentByName(const std::string &name);

        void GetAllComponent(SayNoArray<Component *> &mConponentArray);

    protected:
        void OnDestory() override;

    public:
        inline const std::string &GetBindAddress() { return this->mSessionAddress; }

        inline const long long GetGameObjectID() const { return this->mGameObjectId; }

    private:
        void PollComponent(float t);

    private:
        long long mGameObjectId;
        std::string mSessionAddress;
        SayNoQueue<Component *> mWaitStartComponents;
        SayNoHashMap<std::string, Component *> mComponentMap;
    private:
        typedef SayNoHashMap<std::string, Component *>::iterator ComponentIter;
    };

    typedef shared_ptr<GameObject> SharedGameObject;

    template<typename T>
    inline T *GameObject::GetComponent()
    {
        std::string name;
        if (!Sentry::GetTypeName<T>(name))
        {
            SayNoDebugError("use 'TYPE_REFLECTION' register type:" << typeid(T).name());
            return false;
        }

        auto iter = this->mComponentMap.find(name);
        if (iter != this->mComponentMap.end())
        {
            Component *component = iter->second;
            return static_cast<T *>(component);
        }
        auto iter1 = this->mComponentMap.begin();
        for (; iter1 != this->mComponentMap.end(); iter1++)
        {
            T *component = dynamic_cast<T *>(iter->second);
            if (component != nullptr)
            {
                return component;
            }
        }
        return nullptr;
    }

    template<typename T>
    inline T *GameObject::AddComponent()
    {
        std::string name;
        if (!Sentry::GetTypeName<T>(name))
        {
            SayNoDebugError("use 'TYPE_REFLECTION' register type:" << typeid(T).name());
            return nullptr;
        }
        Component *component = this->GetComponent<T>();
        if (component == nullptr)
        {
            component = new T(this->shared_from_this());
            if (component != nullptr)
            {
                this->mWaitStartComponents.push(component);
                this->mComponentMap.insert(std::make_pair(name, component));
            }
        }
        return static_cast<T *>(component);
    }

    template<typename T>
    inline bool GameObject::RemoveComponent()
    {
        std::string name;
        if (!Sentry::GetTypeName<T>(name))
        {
            return false;
        }
        return this->RemoveComponent(name);
    }

    template<typename T>
    inline T *GameObject::GetOrAddComponent()
    {
        T *component = this->GetComponent<T>();
        return component == nullptr ? this->AddComponent<T>() : component;
    }
}
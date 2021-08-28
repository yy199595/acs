#pragma once

#include <Define/ClassStatement.h>
#include <Define/CommonDef.h>
#include <Define/CommonTypeDef.h>

namespace Sentry
{
    class Object
    {
    public:
        Object();

        virtual ~Object();

    public:
        bool Init(const std::string &name);

    public:
        inline bool IsActive() { return this->mIsActive; }

        inline long long GetIntanceID() { return mIntanceID; }
        inline void SetActive(bool isActive) { this->mIsActive = isActive; }

        inline const std::string &GetTypeName() { return this->mClassName; }

    public:
        virtual bool IsManager() { return false; }

        virtual bool IsComponent() { return false; }

        virtual bool IsGameObejct() { return false; }

        virtual void OnDestory() {}

    private:
        bool mIsActive;
        long long mIntanceID;
        std::string mClassName;
    };
}// namespace Sentry

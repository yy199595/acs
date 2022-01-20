#pragma once

#include <Define/ClassStatement.h>
#include <Define/CommonLogDef.h>
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
        inline bool IsActive() const { return this->mIsActive; }

        inline void SetActive(bool isActive) { this->mIsActive = isActive; }

        inline const std::string &GetTypeName() { return this->mClassName; }

    public:
        virtual bool IsManager() { return false; }

        virtual bool IsComponent() { return false; }

        virtual bool IsGameObject() { return false; }

        virtual void OnDestory() {}

    private:
        bool mIsActive;
        std::string mClassName;
    };
}// namespace Sentry

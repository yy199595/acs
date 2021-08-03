#pragma once

#include<Define/CommonDef.h>
#include<Define/CommonTypeDef.h>
#include<Define/ClassStatement.h>
#include<Core/Applocation.h>

namespace Sentry
{
    class Object
    {
    public:
        Object();

        virtual ~Object();

    public:
        bool Init(Applocation *app, const std::string &name);

    public:
        inline bool IsActive() { return this->mIsActive; }

        inline long long GetIntanceID() { return mIntanceID; }

        inline class Applocation *GetApp() { return mAppLocation; }

        inline void SetActive(bool isActive) { this->mIsActive = isActive; }

        inline const std::string &GetTypeName() { return this->mClassName; }

        inline ServerConfig &GetConfig() { return mAppLocation->GetConfig(); }

    public:
        template<typename T>
        inline T *GetManager() { return mAppLocation->GetManager<T>(); }

    public:
        virtual bool IsManager() { return false; }

        virtual bool IsComponent() { return false; }

        virtual bool IsGameObejct() { return false; }

        virtual void OnDestory() {}

    private:
        bool mIsActive;
        long long mIntanceID;
        std::string mClassName;

        class Applocation *mAppLocation;
    };
}


#pragma once

#include"Async/TaskSource.h"
#include<Component/Component.h>
#include<Script/ClassProxyHelper.h>
#include<Script/LuaTable.h>

namespace Sentry
{
    class LuaScriptComponent : public Component, public IStart
    {
    public:
        LuaScriptComponent() = default;

		virtual ~LuaScriptComponent() = default;

    public:
        TaskSource<void> * Invoke(int ref);

        struct lua_State *GetLuaEnv() { return this->mLuaEnv; }

    public:
        template<typename T>
        bool PushGlobalPoint(const std::string name, T *data);

        inline std::string GetMainPath() { return this->mMainLuaPath; }

    public:
        bool GetLuaTable(const std::string & name);
        bool GetLuaFunction(const std::string & tab, const std::string & func);
    protected:
        bool Awake() final;
        void OnStart() final;
        bool LateAwake() final;
        void OnDestory() final;

    private:
		bool LoadAllFile();

        void ClearRequirePath();

        void AddRequirePath(const std::string path);

		bool LoadLuaScript(const std::string filePath);
    private:

        void OnPushGlobalObject();

        void PushClassToLua();

    private:
        void RegisterExtension();

    private:
        std::string mMainLuaPath;
        struct lua_State *mLuaEnv;
        const static std::string mName;
        std::vector<std::string> mRequirePaths;
        std::unordered_map<std::string, int> mGlobalRefMap;
		std::unordered_map<std::string, std::string> mLuaFileMd5s;
    };

    template<typename T>
    inline bool LuaScriptComponent::PushGlobalPoint(const std::string name, T *data)
    {
        const char *mateName = ClassNameProxy::GetLuaClassName<T>();
        if (mateName != nullptr)
        {
            PtrProxy<T>::Write(mLuaEnv, data);
            lua_getglobal(mLuaEnv, mateName);
            if (lua_istable(mLuaEnv, -1))
            {
                lua_setmetatable(mLuaEnv, -2);
                lua_setglobal(mLuaEnv, name.c_str());
                return true;
            }
        }
        return false;
    }
}
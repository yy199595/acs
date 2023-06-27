#pragma once
#include<set>
#include<memory>
#include<unordered_map>
#include"Lua/Module/LuaModule.h"
#include"Lua/Config/LuaConfig.h"
#include"Entity/Component/Component.h"
struct lua_State;
namespace Tendo
{
    class LuaComponent final : public Component, public IStart,
						 public IComplete, public IHotfix, public ISecondUpdate,
						 public IServerRecord, public IDestroy
	{
	 public:
		LuaComponent();
		virtual ~LuaComponent() final = default;
    public:
		double GetMemorySize();
		double CollectGarbage();
		Lua::LuaModule * LoadModule(const std::string & name);
		struct lua_State* GetLuaEnv() { return this->mLuaEnv; }
		bool GetFunction(const std::string& tab, const std::string& func);
	 protected:
		bool Awake() final;
		void Start() final;
		bool LateAwake() final;
		void OnDestroy() final;
		void Complete() final;
		bool OnHotFix() final;
        void OnSecondUpdate(int tick) final;
		void OnRecord(Json::Writer &document) final;
    private:
		bool LoadAllFile();
		void RegisterLuaClass();
        void AddRequire(const std::string & direct);
		Lua::LuaModule* LoadModuleByPath(const std::string& name);
	 private:
        lua_State* mLuaEnv;
		std::string mModulePath;
		std::string mComponentPath;
		Lua::LuaModule * mMainModule;
		std::unique_ptr<LuaConfig> mLuaConfig;
	};
}
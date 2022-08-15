//
// Created by zmhy0073 on 2022/8/15.
//

#ifndef APP_LOCALTABLE_H
#define APP_LOCALTABLE_H
#include"LuaInclude.h"

namespace Lua
{
    class LocalTable
    {
    public:
        LocalTable(lua_State * lua);
        ~LocalTable();
    public:
        void Clear();
        bool Load(const std::string & path);
        bool GetFunction(const std::string & func);
        const std::string & GetName() const { return this->mTableName; }
    private:
        int mIndex;
        std::string mMd5;
        lua_State * mLua;
        std::string mPath;
        std::string mTableName;
        std::unordered_map<std::string, int> mIndexMap;
    };
}


#endif //APP_LOCALTABLE_H

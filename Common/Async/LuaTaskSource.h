//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef GAMEKEEPER_LUATASKSOURCE_H
#define GAMEKEEPER_LUATASKSOURCE_H
#include"TaskSource.h"
namespace GameKeeper
{
    class LuaTaskSource
    {
    public:
        LuaTaskSource() = default;
        ~LuaTaskSource() = default;
    public:
        void SetResult(int code, std::string json);
    public:
        XCode Await();
        const std::string & GetJson() { return this->mJson;}
    private:
        std::string mJson;
        TaskSource<XCode> mTaskSource;
    };
}
#endif //GAMEKEEPER_LUATASKSOURCE_H

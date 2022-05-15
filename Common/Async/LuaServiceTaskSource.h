//
// Created by zmhy0073 on 2022/1/8.
//

#ifndef GAMEKEEPER_LUATASKSOURCE_H
#define GAMEKEEPER_LUATASKSOURCE_H
#include"TaskSource.h"

namespace Sentry
{
    class LuaServiceTaskSource final
    {
    public:
        LuaServiceTaskSource();
        ~LuaServiceTaskSource() = default;
    public:
		void SetError(const std::string & error);
        void SetResult(int code, std::string & json);
    public:
        XCode Await();
        const std::string & GetJson() { return this->mJson;}
    private:
        XCode mCode;
        std::string mJson;
        TaskSource<XCode> mTaskSource;
    };
}
#endif //GAMEKEEPER_LUATASKSOURCE_H

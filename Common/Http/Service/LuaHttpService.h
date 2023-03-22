//
// Created by zmhy0073 on 2022/6/6.
//

#ifndef SERVER_LOCALLUAHTTPSERVICE_H
#define SERVER_LOCALLUAHTTPSERVICE_H
#include"LocalHttpService.h"
namespace Sentry
{
    class LuaHttpService : public LocalHttpService
    {
    public:
        LuaHttpService();
        ~LuaHttpService() = default;
    private:
		bool OnInit() final;
		bool OnStart() final { return true; }
		bool OnClose() final { return true; }
    private:
        class LuaScriptComponent* mLuaComponent;
    };
}


#endif //SERVER_LOCALLUAHTTPSERVICE_H

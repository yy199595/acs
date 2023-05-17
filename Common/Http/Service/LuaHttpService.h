//
// Created by zmhy0073 on 2022/6/6.
//

#ifndef SERVER_LOCALLUAHTTPSERVICE_H
#define SERVER_LOCALLUAHTTPSERVICE_H
#include"HttpService.h"
namespace Tendo
{
    class LuaHttpService : public HttpService
    {
    private:
		bool OnInit() final { return this->GetLuaModule() != nullptr; }
    };
}


#endif //SERVER_LOCALLUAHTTPSERVICE_H

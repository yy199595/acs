//
// Created by zmhy0073 on 2022/6/6.
//

#ifndef APP_LUAHTTPSERVICE_H
#define APP_LUAHTTPSERVICE_H
#include"HttpService.h"
namespace acs
{
    class LuaHttpService final : public HttpService
    {
    private:
		bool OnInit() final { return this->GetLuaModule() != nullptr; }
    };
}


#endif //APP_LUAHTTPSERVICE_H

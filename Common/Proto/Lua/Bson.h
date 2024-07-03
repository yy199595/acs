//
// Created by leyi on 2023/12/14.
//

#ifndef APP_BSON_H
#define APP_BSON_H
#include"Lua/Engine/Define.h"

namespace lua
{
	namespace lbson
	{
		extern int encode(lua_State * L);
		extern int decode(lua_State * L);
	}
}


#endif //APP_BSON_H

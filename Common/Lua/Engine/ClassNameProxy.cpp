#include "ClassNameProxy.h"

namespace Lua
{
	namespace ClassNameProxy
	{
		std::unordered_map<size_t, ClassRegisterInfo*> classNameMap = {};
	}
}
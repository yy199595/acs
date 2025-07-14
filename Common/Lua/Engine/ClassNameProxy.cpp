#include "ClassNameProxy.h"

namespace Lua
{
	namespace ClassNameProxy
	{
		std::unordered_map<size_t, std::unique_ptr<ClassRegisterInfo>> classNameMap = {};
	}
}
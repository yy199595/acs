#include "ObjectFactory.h"
#include <Define/CommonLogDef.h>

namespace GameKeeper
{
    ObjectFactory *ObjectFactory::Get()
    {
        static ObjectFactory factory(10);
        return &factory;
    }
}// namespace GameKeeper

#include "ObjectFactory.h"
#include <Define/CommonDef.h>

namespace GameKeeper
{
    ObjectFactory *ObjectFactory::Get()
    {
        static ObjectFactory factory(10);
        return &factory;
    }
}// namespace GameKeeper

#include "ObjectFactory.h"
#include <Define/CommonDef.h>

namespace Sentry
{
    ObjectFactory *ObjectFactory::Get()
    {
        static ObjectFactory factory(10);
        return &factory;
    }
}

#include "ObjectFactory.h"
#include <Define/CommonLogDef.h>

namespace Sentry
{
    ObjectFactory *ObjectFactory::Get()
    {
        static ObjectFactory factory(10);
        return &factory;
    }
}// namespace Sentry

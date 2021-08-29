#include"Component.h"
namespace Sentry
{
	Component::Component()
		: gameObject(nullptr)
	{
		this->mType = nullptr;
		this->gameObjectID = 0;
	}
}
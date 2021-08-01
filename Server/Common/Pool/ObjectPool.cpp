#include"ObjectPool.h"

namespace Sentry
{
	thread_local ObjectPool<NetMessageProxy> GnetPacketPool;
}
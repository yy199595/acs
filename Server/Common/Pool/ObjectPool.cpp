#include"ObjectPool.h"

namespace Sentry
{
	thread_local ObjectPool<com::NetWorkPacket> GnetPacketPool;
}
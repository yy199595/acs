#include"ObjectPool.h"

namespace SoEasy
{
	thread_local ObjectPool<PB::NetWorkPacket> GnetPacketPool;
}
#include<queue>
#include"Message/s2s.pb.h"
#include"Entity/Component/Component.h"

namespace Tendo
{
	class WatchDogComponent final : public Component, public ISecondUpdate
	{
	public:
		WatchDogComponent() = default;
	private:
		bool LateAwake() final;
		void OnSecondUpdate(int tick) final;
	private:
		std::string mAddress;
	};
}
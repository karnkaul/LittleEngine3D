#pragma once
#include "le3d/core/delegate.hpp"
#include "le3d/game/ecs/system.hpp"

namespace le
{
class FreeCamController : public System
{
public:
	using ForEachCam = Delegate<class CFreeCam&>;

protected:
	ForEachCam m_forEachCam;

public:
	ForEachCam::Token setCallback(ForEachCam::Callback callback);

protected:
	void tick(class ECSDB& db, Time dt) override;
};
} // namespace le

#pragma once
#include "../../valve_sdk/csgostructs.hpp"

enum class YawAntiAims : int
{
	NONE,
	BACKWARDS,
	SPINBOT,
	LOWER_BODY,
	FREESTANDING
};

enum class YawAddAntiAims : int
{
	NONE,
	JITTER,
	SWITCH,
	SPIN,
	RANDOM
};

enum class PitchAntiAims : int
{
	NONE,
	EMOTION,
	DOWN,
	UP,
	ZERO
};

class CAntiAim : public Singleton<CAntiAim>
{
public:
	void CreateMove(CUserCmd* cmd, bool& bSendPacket);
	void LbyBreakerPrediction(CUserCmd* cmd, bool& bSendPacket);
	void ResetLbyPrediction();

	void Fakewalk(CUserCmd* cmd, bool& bSendPackets);

	bool GetEdgeDetectAngle(C_BasePlayer* entity, float& yaw);

	void SlideWalk(CUserCmd* cmd);
private:
	void DoAntiAim(CUserCmd* cmd, bool& bSendPacket);
	void Pitch(CUserCmd* cmd);
	void Yaw(CUserCmd* cmd, bool fake);
	void YawAdd(CUserCmd* cmd, bool fake);
	int GetFPS();

	//freestanding
	float fov_player(Vector ViewOffSet, QAngle View, C_BasePlayer* entity);
	int GetNearestPlayerToCrosshair();
	bool Freestanding(C_BasePlayer* player, float& ang);
	bool FreestandingLbyBreak(float& ang);

	bool InLbyUpdate = false;
	bool NextTickInLbyUpdate = false;
	float NextLbyUpdate = 0.f;

	CBaseHandle* m_ulEntHandle = nullptr;
	CCSGOPlayerAnimState* m_serverAnimState = nullptr;

	bool allocate = false, change = false, reset = false;

	float GetMaxDesyncYaw();
};
#pragma once
#include "../../valve_sdk/csgostructs.hpp"
#include "../../hooks.hpp"

class CMisc : public Singleton<CMisc>
{
public:
	void SetNorecoilAngles(ClientFrameStage_t stage, QAngle* aim_punch, QAngle* view_punch, QAngle old_aim_punch, QAngle old_view_punch);
	void NoVisualRecoil(ClientFrameStage_t stage, QAngle* aim_punch, QAngle* view_punch, QAngle old_aim_punch, QAngle old_view_punch);
	void SetThirdpersonAngles(ClientFrameStage_t stage);
	void FakeLag(CUserCmd * cmd, bool & bSendPacket);
	void SlowWalk(CUserCmd * cmd);
	void ClanTag();
	void AutoStrafe(CUserCmd * cmd);
	void OverrideView(CViewSetup * vsView);
	void FastDuck(CUserCmd * cmd);
	void Bhop(CUserCmd* cmd);
	bool AntiKick(Hooks::tDispatchUserMessage oDispatchUserMessage, PVOID ecx, int type, unsigned int a3, unsigned int length, const void *msg_data);
	void AutoAccept(const char* pSoundEntry);
};
#pragma once
#include "../../options.hpp"
#include "../../valve_sdk/csgostructs.hpp"

class CLegitBot : public Singleton<CLegitBot>
{
public:
	float GetFov(/*float distance*/);
	void Aimbot(CUserCmd *pCmd, bool bSendPacket);
	void TriggerBot(CUserCmd * cmd);
	bool IsEnabled(CUserCmd *pCmd);
private:
	bool IsRcs();
	float GetSmooth();
	void RCS(QAngle &angle, C_BasePlayer* target);
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	bool IsNotSilent(float fov);
	C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, int &bestBone);
	float shot_delay_time;
	bool shot_delay = false;
	bool silent_enabled = false;
	QAngle CurrentPunch = { 0,0,0 };
	QAngle RCSLastPunch = { 0,0,0 };
	bool is_delayed = false;
	int kill_delay_time;
	bool kill_delay = false;
	C_BasePlayer* target = NULL;
};
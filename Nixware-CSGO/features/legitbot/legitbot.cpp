#include "legitbot.hpp"
#include "../../helpers/math.hpp"
#include "../../helpers/draw_manager.hpp"
#include <chrono>

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

bool CLegitBot::IsRcs()
{
	return Globals::LocalPlayer->m_iShotsFired() > 1;
}

bool CLegitBot::IsEnabled(CUserCmd *pCmd)
{
	if (!Utils::IsInGame()) 
		return false;

	if (!Globals::LocalPlayer)
		return false;

	if (!Globals::LocalPlayer->IsAlive())
		return false;
	
	auto pWeapon = Globals::LocalPlayer->m_hActiveWeapon();
	if (!pWeapon || !(pWeapon->IsSniper() || pWeapon->IsPistol() || pWeapon->IsAutomaticGun())) {
		return false;
	}
	auto weaponData = pWeapon->GetCSWeaponData();
	auto weapontype = weaponData->iWeaponType;

	if (!WeaponSettings->aim_enabled || !(pCmd->buttons & IN_ATTACK)) {
		return false;
	}

	if (!Utils::HitChance(WeaponSettings->aim_hitchance))
		return false;

	if ((pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08) && WeaponSettings->aim_only_in_zoom && !Globals::LocalPlayer->m_bIsScoped()) {
		return false;
	}
	if (WeaponSettings->aim_fov == 0 && WeaponSettings->aim_psilent_fov == 0 && !WeaponSettings->aim_rcs) {
		return false;
	}
	if (!pWeapon->HasBullets() || pWeapon->IsReloading()) {
		return false;
	}
	return true;
}

float CLegitBot::GetSmooth()
{
	if (IsRcs() && Globals::LocalPlayer->m_hActiveWeapon()->IsAutomaticGun())
		return WeaponSettings->aim_smooth_2;
	else
		return WeaponSettings->aim_smooth;
}

void CLegitBot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle)
{
	if (GetSmooth() <= 1) {
		return;
	}
	Vector vAimAngle;
	Math::AngleVectors(aimAngle, vAimAngle);
	Vector vCurrentAngle;
	Math::AngleVectors(currentAngle, vCurrentAngle);
	Vector delta = vAimAngle - vCurrentAngle;
	Vector smoothed = vCurrentAngle + delta / GetSmooth();
	Math::VectorAngles(smoothed, angle);
}

void CLegitBot::RCS(QAngle &angle, C_BasePlayer* target)
{
	if (!WeaponSettings->aim_rcs || (!Globals::LocalPlayer->m_hActiveWeapon()->IsPistol() && !IsRcs())) {
		return;
	}
	if (WeaponSettings->aim_rcs_x == 0 && WeaponSettings->aim_rcs_y == 0) {
		return;
	}
	if (target) {
		QAngle punch = Globals::LocalPlayer->m_aimPunchAngle();
		float x = WeaponSettings->aim_rcs_x * 0.02f;
		float y = WeaponSettings->aim_rcs_y * 0.02f;

		angle.pitch -= punch.pitch * x;
		angle.yaw -= punch.yaw * y;
	}
	Math::Normalize3(angle);
	Math::ClampAngles(angle);
}

float CLegitBot::GetFov()
{
	if (IsRcs() && Globals::LocalPlayer->m_hActiveWeapon()->IsAutomaticGun())
		return WeaponSettings->aim_fov_2;

	if (WeaponSettings->aim_perfect_silent)
		return WeaponSettings->aim_psilent_fov;

	return WeaponSettings->aim_fov;
}

C_BasePlayer* CLegitBot::GetClosestPlayer(CUserCmd* cmd, int &bestBone)
{
	QAngle ang;
	Vector eVecTarget;
	Vector pVecTarget = Globals::LocalPlayer->GetEyePos();
	if (target && !kill_delay && WeaponSettings->aim_kill_delay > 0 && target->IsNotTarget()) {
		target = NULL;
		shot_delay = false;
		kill_delay = true;
		kill_delay_time = (int)GetTickCount() + WeaponSettings->aim_kill_delay;
	}
	if (kill_delay) {
		if (kill_delay_time <= (int)GetTickCount()) kill_delay = false;
		else return NULL;
	}
	C_BasePlayer* player;
	target = NULL;
	int bestHealth = 100.f;
	float bestFov = 9999.f;
	float bestDamage = 0.f;
	float bestBoneFov = 9999.f;
	float bestDistance = 9999.f;
	float fov;
	float distance;
	int fromBone = WeaponSettings->aim_nearest ? 0 : WeaponSettings->aim_hitbox;
	int toBone = WeaponSettings->aim_nearest ? 7 : WeaponSettings->aim_hitbox;

	for (int i = 1; i < Interfaces::Engine->GetMaxClients(); ++i)
	{
		player = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(i);
		if (player->IsNotTarget()) {
			continue;
		}
		if (Vars.legit_ignoreteam && player->IsTeammate()) {
			continue;
		}
		for (int bone = fromBone; bone <= toBone; bone++) {
			eVecTarget = player->GetHitboxPos(bone);
			Math::VectorAngles(eVecTarget - pVecTarget, ang);
			Math::Normalize3(ang);
			Math::ClampAngles(ang);

			distance = pVecTarget.DistTo(eVecTarget);
		
			QAngle p = WeaponSettings->aim_recoil_based_fov ? RCSLastPunch * 2.f : RCSLastPunch;

			fov = Utils::GetFovToPlayer(cmd->viewangles + p, ang);

			if (fov > GetFov()) {
				continue;
			}
			if (!WeaponSettings->aim_ignore_walls && !Globals::LocalPlayer->CanSeePlayer(player, eVecTarget)) continue;

			if (Vars.legit_smokecheck && Utils::LineGoesThroughSmoke(pVecTarget, eVecTarget)) {
				continue;
			}
			if (WeaponSettings->aim_nearest == 1 && bestBoneFov < fov) {
				continue;
			}
			bestBoneFov = fov;

			if (bestFov > fov) {
				bestBone = bone;
				target = player;
				bestFov = fov;
				bestDistance = distance;
			}
		}
	}
	return target;
}

bool CLegitBot::IsNotSilent(float fov)
{
	return IsRcs() || !WeaponSettings->aim_perfect_silent;
}

void CLegitBot::Aimbot(CUserCmd *pCmd, bool bSendPacket)
{
	if (!Vars.legit_enable)
		return;

	if (!IsEnabled(pCmd)) {
		RCSLastPunch = { 0, 0, 0 };
		is_delayed = false;
		shot_delay = false;
		kill_delay = false;
		target = NULL;
		return;
	}

	QAngle angles = pCmd->viewangles; //
	QAngle current = angles;
	float fov = 180.f;
	if (!(Vars.legit_flashcheck && Globals::LocalPlayer->IsFlashed())) 
	{
		int bestBone = -1;
		if (GetClosestPlayer(pCmd, bestBone)) {
			Math::VectorAngles(target->GetHitboxPos(bestBone) - Globals::LocalPlayer->GetEyePos(), angles);
			Math::Normalize3(angles);
			Math::ClampAngles(angles);

			fov = Utils::GetFovToPlayer(pCmd->viewangles, angles);

			if (!WeaponSettings->aim_perfect_silent && !is_delayed && !shot_delay && WeaponSettings->aim_shot_delay > 0 && Globals::LocalPlayer->m_iShotsFired() < 1) {
				is_delayed = true;
				shot_delay = true;
				shot_delay_time = GetTickCount() + WeaponSettings->aim_shot_delay;
			}
			if (shot_delay && shot_delay_time <= GetTickCount()) {
				shot_delay = false;
			}
			if (shot_delay) {
				pCmd->buttons &= ~IN_ATTACK;
			}
		}
	}
	CurrentPunch = Globals::LocalPlayer->m_aimPunchAngle();
	if (IsNotSilent(fov)) {
		RCS(angles, target);
	}

	RCSLastPunch = CurrentPunch;

	if (target && IsNotSilent(fov)) {
		Smooth(current, angles, angles);
	}
	Math::Normalize3(angles);
	Math::ClampAngles(angles);
	pCmd->viewangles = angles;
	if (IsNotSilent(fov)) {
		Interfaces::Engine->SetViewAngles(angles);
	}
	else
	{	
		auto Calced = angles;
		auto Punch = Globals::LocalPlayer->m_aimPunchAngle();
		auto CompensatedCalced = Calced - Punch;

		auto viewangle = pCmd->viewangles;

		auto delta = Calced - viewangle;
		auto compensated_delta = CompensatedCalced - viewangle;

		delta.Normalize();
		compensated_delta.Normalize();

		if (delta.IsZero() || compensated_delta.IsZero())
			return;

		if ((Globals::LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - (Globals::LocalPlayer->m_nTickBase() * Interfaces::GlobalVars->interval_per_tick)) < 0)
		{
			int tick = -1; tick++;

			if (tick < 1)
			{
				bSendPacket = false;
				pCmd->viewangles += delta;
			}
			else
			{
				bSendPacket = true;
				tick = -1;
			}
		}
		else
		{
			bSendPacket = true;
		}
	}
	if (Globals::LocalPlayer->m_hActiveWeapon()->IsPistol() && WeaponSettings->aim_autopistol) {
		float server_time = Globals::LocalPlayer->m_nTickBase() * Interfaces::GlobalVars->interval_per_tick;
		float next_shot = Globals::LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;
		if (next_shot > 0) {
			pCmd->buttons &= ~IN_ATTACK;
		}
	}
}

long getMils()
{
	auto duration = std::chrono::system_clock::now().time_since_epoch();

	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void CLegitBot::TriggerBot(CUserCmd* cmd)
{
	auto Key = Vars.legit_trigger_key;

	if (!WeaponSettings->trigger_enable)
		return;

	if (Key < 1)
	{
		Vars.trigger_keypressed = true;
	}
	else if (Key > 0 && GetAsyncKeyState(Key))
	{
		Vars.trigger_keypressed = true;
	}
	else if (Key > 0 && !GetAsyncKeyState(Key))
	{
		Vars.trigger_keypressed = false;
	}

	if (!Vars.trigger_keypressed)
		return;

	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
		return;

	if (!Utils::HitChance(WeaponSettings->trigger_hitchance))
		return;

	if (Vars.legit_flashcheck && Globals::LocalPlayer->IsFlashed())
		return;

	//if ((Vars.trigger_jump && !(Globals::LocalPlayer->m_fFlags() & FL_ONGROUND)))
	//	return;

	long currentTime_ms = getMils();
	static long timeStamp = currentTime_ms;
	long oldTimeStamp;

	Vector traceStart, traceEnd;
	trace_t tr;

	QAngle viewAngles;
	Interfaces::Engine->GetViewAngles(viewAngles);
	QAngle viewAngles_rcs = viewAngles + Globals::LocalPlayer->m_aimPunchAngle() * 2.0f;

	Math::AngleVectors(viewAngles_rcs, traceEnd);

	traceStart = Globals::LocalPlayer->GetEyePos();
	traceEnd = traceStart + (traceEnd * 8192.0f);

	Utils::TraceLine(traceStart, traceEnd, 0x46004003, Globals::LocalPlayer, &tr);

	oldTimeStamp = timeStamp;
	timeStamp = currentTime_ms;

	C_BasePlayer* Player = (C_BasePlayer*)tr.hit_entity;
	if (!Player)
		return;

	if (Player->GetClientClass()->m_ClassID != CCSPlayer)
		return;

	if (Player == Globals::LocalPlayer || Player->IsDormant() || !Player->IsAlive() || Player->m_bGunGameImmunity())
		return;

	if (Player->IsTeammate() && Vars.legit_ignoreteam)
		return;

	if (!((WeaponSettings->trigger_Head && tr.hitgroup == HITGROUP_HEAD)
		|| (WeaponSettings->trigger_Body && (tr.hitgroup == HITGROUP_CHEST
			|| tr.hitgroup == HITGROUP_STOMACH)) || (WeaponSettings->trigger_Misc && (tr.hitgroup >= HITGROUP_LEFTARM && tr.hitgroup <= HITGROUP_RIGHTLEG))))
		return;

	if (Vars.legit_smokecheck &&  Utils::LineGoesThroughSmoke(Globals::LocalPlayer->GetEyePos(), tr.endpos))
		return;

	C_BaseCombatWeapon* activeWeapon = Globals::LocalPlayer->m_hActiveWeapon();

	if (!activeWeapon || !activeWeapon->HasBullets())
		return;

	int itemDefinitionIndex = activeWeapon->m_iItemDefinitionIndex();

	if (itemDefinitionIndex == WEAPON_KNIFE || itemDefinitionIndex >= WEAPON_KNIFE_BAYONET)
		return;

	int weaponType = activeWeapon->GetCSWeaponData()->iWeaponType;
	if (weaponType == WEAPONTYPE_C4 || weaponType == WEAPONTYPE_GRENADE)
		return;

	if (activeWeapon->m_flNextPrimaryAttack() > Interfaces::GlobalVars->curtime)
	{
		if (activeWeapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			cmd->buttons &= ~IN_ATTACK2;
		}
		else
		{
			cmd->buttons &= ~IN_ATTACK;
		}
	}
	else
	{
		if ((WeaponSettings->trigger_delay > 0) && currentTime_ms - oldTimeStamp < (WeaponSettings->trigger_delay * 10))
		{
			timeStamp = oldTimeStamp;
			return;
		}

		if (activeWeapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			cmd->buttons |= IN_ATTACK2;
		}
		else
		{
			cmd->buttons |= IN_ATTACK;
		}
	}

	timeStamp = currentTime_ms;
}

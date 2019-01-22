#include "antiaim.hpp"
#include "../../options.hpp"
#include "../../helpers/math.hpp"

void CAntiAim::CreateMove(CUserCmd* cmd, bool& bSendPacket)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		return;
	}

	int movetype = g_LocalPlayer->m_nMoveType();

	if (
		movetype == MOVETYPE_FLY
		|| movetype == MOVETYPE_NOCLIP
		|| cmd->buttons & IN_USE
		|| cmd->buttons & IN_GRENADE1
		|| cmd->buttons & IN_GRENADE2
		)
	{
		return;
	}

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
	{
		return;
	}

	if (weapon->m_flNextPrimaryAttack() - g_GlobalVars->curtime < g_GlobalVars->interval_per_tick && (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2))
	{
		return;
	}

	if (movetype == MOVETYPE_LADDER)
	{
		static bool last = false;
		bSendPacket = last;
		last = !last;
		return;
	}

	if (weapon->IsGrenade() && weapon->m_fThrowTime() > 0.1f)
	{
		bSendPacket = false;
		return;
	}

	DoAntiAim(cmd, bSendPacket);
}

void CAntiAim::DoAntiAim(CUserCmd* cmd, bool& bSendPacket)
{
	Yaw(cmd, false);
	Pitch(cmd);

	if (Vars.ragebot_antiaim_desync)
	{
		bool Moving = g_LocalPlayer->m_vecVelocity().Length2D() > 0.1f || (cmd->sidemove != 0.f || cmd->forwardmove != 0.f);
		bool InAir = !(g_LocalPlayer->m_fFlags() & FL_ONGROUND);
		bool Standing = !Moving && !InAir;

		int FakeLagTicks = Vars.ragebot_fakelag_amt;

		if (FakeLagTicks == 0)
		{
			static bool sw = false;
			bSendPacket = sw;
			sw = !sw;
		}

		static QAngle LastRealAngle = QAngle(0, 0, 0);

		if (!bSendPacket && !(cmd->buttons & IN_ATTACK))
		{
			static bool bFlip = false;
			cmd->viewangles.yaw += bFlip ? 58.f : -58.f;
		}

		if (bSendPacket)
		{
			LastRealAngle = cmd->viewangles;
		}
	}
}

void CAntiAim::Pitch(CUserCmd* cmd)
{
	bool Moving = g_LocalPlayer->m_vecVelocity().Length2D() > 0.1;
	bool InAir = !(g_LocalPlayer->m_fFlags() & FL_ONGROUND);
	bool Standing = !Moving && !InAir;
	PitchAntiAims mode = (PitchAntiAims)Vars.ragebot_antiaim_pitch;

	float CustomPitch = 0.f;

	switch (mode)
	{
	case PitchAntiAims::EMOTION:
		cmd->viewangles.pitch = 82.f;
		break;

	case PitchAntiAims::DOWN:
		cmd->viewangles.pitch = 90.f;
		break;

	case PitchAntiAims::UP:
		cmd->viewangles.pitch = -90.f;
		break;

	case PitchAntiAims::ZERO:
		cmd->viewangles.pitch = 0.f;
		break;
	}
}

void CAntiAim::Yaw(CUserCmd* cmd, bool fake)
{
	bool Moving = g_LocalPlayer->m_vecVelocity().Length2D() > 0.1;
	bool InAir = !(g_LocalPlayer->m_fFlags() & FL_ONGROUND);
	bool Standing = !Moving && !InAir;
	YawAntiAims mode = (YawAntiAims)Vars.ragebot_antiaim_yaw;

	float CustomYaw = 0.f;

	switch (mode)
	{
	case YawAntiAims::BACKWARDS:
		cmd->viewangles.yaw -= 180.f;
		break;

	case YawAntiAims::SPINBOT:
		cmd->viewangles.yaw = fmodf(g_GlobalVars->tickcount * 10.f, 360.f);
		break;

	case YawAntiAims::LOWER_BODY:
		cmd->viewangles.yaw = g_LocalPlayer->m_flLowerBodyYawTarget();
		break;
	case YawAntiAims::FREESTANDING:
	{
		float ang = 0.f;
		bool canuse = Freestanding(g_LocalPlayer, ang);

		if (!canuse)
		{ 
			cmd->viewangles.yaw -= 180.f;
		}
		else
		{
			cmd->viewangles.yaw = ang;
		}

		break;
	}
	}
}

bool CAntiAim::Freestanding(C_BasePlayer* player, float& ang)
{
	if (!g_LocalPlayer || !player || !player->IsAlive() || !g_LocalPlayer->IsAlive())
	{
		return false;
	}

	C_BasePlayer* local = g_LocalPlayer;

	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	static float hold = 0.f;
	Vector besthead;

	auto leyepos = local->m_vecOrigin() + local->m_vecViewOffset();
	auto headpos = local->GetHitboxPos(0); //GetHitboxPosition(local_player, 0);
	auto origin = local->m_vecOrigin();

	auto checkWallThickness = [&](C_BasePlayer * pPlayer, Vector newhead) -> float
	{

		Vector endpos1, endpos2;

		Vector eyepos = pPlayer->m_vecOrigin() + pPlayer->m_vecViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, local);

		trace_t trace1, trace2;
		g_EngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &trace1);

		if (trace1.DidHit())
		{
			endpos1 = trace1.endpos;
		}
		else
		{
			return 0.f;
		}

		ray.Init(eyepos, newhead);
		g_EngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &trace2);

		if (trace2.DidHit())
		{
			endpos2 = trace2.endpos;
		}

		float add = newhead.DistTo(eyepos) - leyepos.DistTo(eyepos) + 3.f;
		return endpos1.DistTo(endpos2) + add / 3;

	};

	int index = GetNearestPlayerToCrosshair();
	static C_BasePlayer* entity;

	if (!local->IsAlive())
	{
		hold = 0.f;
	}

	if (index != -1)
	{
		entity = (C_BasePlayer*)g_EntityList->GetClientEntity(index); // maybe?
	}

	if (!entity || entity == nullptr)
	{
		return false;
	}

	float radius = Vector(headpos - origin).Length2D();

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		float step = (M_PI * 2) / 90;

		for (float besthead = 0; besthead < (M_PI * 2); besthead += step)
		{
			Vector newhead(radius * cos(besthead) + leyepos.x, radius * sin(besthead) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			no_active = false;
			totalthickness += checkWallThickness(entity, newhead);

			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;

				bestrotation = besthead;
			}
		}
	}

	if (no_active)
	{
		return false;
	}
	else
	{
		ang = RAD2DEG(bestrotation);
		return true;
	}

	return false;
}

int CAntiAim::GetNearestPlayerToCrosshair()
{
	float BestFov = FLT_MAX;
	int BestEnt = -1;
	QAngle MyAng;
	g_EngineClient->GetViewAngles(MyAng);

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity || !g_LocalPlayer || !entity->IsPlayer() || entity == g_LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || entity->IsTeammate())
		{
			continue;
		}

		float CFov = fov_player(g_LocalPlayer->m_vecOrigin(), MyAng, entity); //Math::GetFOV(MyAng, Math::CalcAngle(g_LocalPlayer->GetEyePos(), entity->GetEyePos()));

		if (CFov < BestFov)
		{
			BestFov = CFov;
			BestEnt = i;
		}
	}

	return BestEnt;
}

void NormalizeNum(Vector& vIn, Vector& vOut)
{
	float flLen = vIn.Length();

	if (flLen == 0)
	{
		vOut.Init(0, 0, 1);
		return;
	}

	flLen = 1 / flLen;
	vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
}

float CAntiAim::fov_player(Vector ViewOffSet, QAngle View, C_BasePlayer* entity)
{
	// Anything past 180 degrees is just going to wrap around
	CONST FLOAT MaxDegrees = 180.0f;

	// Get local angles
	QAngle Angles = View;

	// Get local view / eye position
	Vector Origin = ViewOffSet;

	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	//Vector Origin(0, 0, 0);
	Vector Forward(0, 0, 0);

	// Convert angles to normalized directional forward vector
	Math::AngleVectors(Angles, Forward);

	Vector AimPos = entity->GetHitboxPos(HITBOX_HEAD); //pvs fix disabled

	//VectorSubtract(AimPos, Origin, Delta);
	Origin.VectorSubtract(AimPos, Origin, Delta);
	//Delta = AimPos - Origin;

	// Normalize our delta vector
	NormalizeNum(Delta, Delta);

	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);

	// Time to calculate the field of view
	return (acos(DotProduct) * (MaxDegrees / M_PI));
}
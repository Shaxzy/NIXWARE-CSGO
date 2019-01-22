#include "ragebot.hpp"
#include "../../helpers/math.hpp"
#include "../../helpers/utils.hpp"
#include "../../options.hpp"
#include "../autowall/autowall.hpp"
#include "../legitbot/legitbot.hpp"
#include "../backtrack/timewarp.hpp"

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) (g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

Vector TickPrediction(Vector AimPoint, C_BasePlayer* pTarget)
{
	return AimPoint + (pTarget->m_vecVelocity() * g_GlobalVars->interval_per_tick);
}

bool CRageBot::HitChance(QAngle angles, C_BasePlayer* ent, float chance)
{
	auto weapon = Globals::LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return false;

	Vector forward, right, up;
	Vector src = Globals::LocalPlayer->GetEyePos();
	Math::AngleVectors(angles, forward, right, up);

	int cHits = 0;
	int cNeededHits = static_cast<int> (150.f * (chance / 100.f));

	weapon->UpdateAccuracyPenalty();
	float weap_spread = weapon->GetSpread();
	float weap_inaccuracy = weapon->GetInaccuracy();

	for (int i = 0; i < 150; i++)
	{
		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, 2.f * PI_F);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, 2.f * PI_F);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->m_Item().m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewAnglesSpread;
		Math::VectorAngles1337(direction, up, viewAnglesSpread);
		viewAnglesSpread.Normalize();

		Vector viewForward;
		Math::AngleVectors(viewAnglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = src + (viewForward * weapon->GetCSWeaponData()->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init(src, viewForward);
		Interfaces::EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, ent, &tr);

		if (tr.hit_entity == ent)
			++cHits;

		if (static_cast<int> ((static_cast<float> (cHits) / 150.f) * 100.f) >= chance)
			return true;

		if ((150 - i + cHits) < cNeededHits)
			return false;
	}

	return false;
}

RbotMatrixData matrixData[128];
int curGroup;

void CRageBot::CreateMove(CUserCmd* cmd, bool& bSendPacket)
{
	static bool DidShotLastTick = false;
	if (!Vars.ragebot_enabled)
		return;

	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive())
		return;

	C_BaseCombatWeapon* weapon = Globals::LocalPlayer->m_hActiveWeapon();

	if (!weapon)
		return;

	CurrentCmd = cmd;

	if (weapon->IsPistol())
		curGroup = WEAPON_GROUPS::PISTOLS;
	else if (weapon->IsRifle() || weapon->IsMashineGun())
		curGroup = WEAPON_GROUPS::RIFLES;
	else if (weapon->IsSMG())
		curGroup = WEAPON_GROUPS::SMG;
	else if (weapon->IsShotgun())
		curGroup = WEAPON_GROUPS::SHOTGUNS;
	else if (weapon->IsAuto())
		curGroup = WEAPON_GROUPS::AUTO;
	else if (weapon->m_iItemDefinitionIndex() == WEAPON_SSG08)
		curGroup = WEAPON_GROUPS::SCOUT;
	else if (weapon->m_iItemDefinitionIndex() == WEAPON_AWP)
		curGroup = WEAPON_GROUPS::AWP;
	else
		curGroup = WEAPON_GROUPS::UNKNOWN;
		
	if (weapon->IsKnife())
		return;

	if (!weapon->HasBullets())
		return;

	Vector hitpos = Vector(0, 0, 0);
	bool bBacktrack = false;
	int BestEntity;
	
	BestEntity = FindBestEntity(cmd, weapon, hitpos, bBacktrack);

	if (hitpos == Vector(0, 0, 0))
		return;

	if (BestEntity == -1)
		return;

	C_BasePlayer* entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(BestEntity));

	Vector predicted = TickPrediction(hitpos, entity);
	QAngle newAng = Math::CalcAngle(Globals::LocalPlayer->GetEyePos(), predicted);
	QAngle a = (Globals::LocalPlayer->m_aimPunchAngle() * 2);
	a.roll = 0.f;
	newAng -= a;
	
	if (weapon->IsSniper() && !Globals::LocalPlayer->m_bIsScoped() && (Globals::LocalPlayer->m_fFlags() & FL_ONGROUND) && Vars.ragebot_autoscope[curGroup])
	{
		if (!(cmd->buttons & IN_ZOOM))
			cmd->buttons |= IN_ZOOM;

		if (Vars.ragebot_autostop[curGroup])
			AutoStop(cmd);

		return;
	}
	float chance = Vars.ragebot_hitchance[curGroup];
	if (!HitChance(newAng, entity, chance))
	{
		if (Vars.ragebot_autostop[curGroup])
			AutoStop(cmd);

		if (Vars.ragebot_autocrouch[curGroup])
			AutoCrouch(cmd);

		return;
	}

	if (!weapon->CanFire() && Interfaces::GlobalVars->curtime <= weapon->m_flNextPrimaryAttack())
		return;

	if (DidShotLastTick && weapon->IsSniper())
	{
		DidShotLastTick = false;
		return;
	}

	LastRbotEnemyIndex = BestEntity;

	if (weapon->IsSniper())
		DidShotLastTick = true;

	Math::NormalizeAngles(newAng);
	Math::ClampAngles(newAng);

	cmd->viewangles = newAng;
	cmd->buttons |= IN_ATTACK;

	//cmd->tick_count = TIME_TO_TICKS(validRecords.at(BestEntity).back().simtime + ÑTimeWarp::Get().GetLerpTime());

	bSendPacket = true;
}

bool CRageBot::InFakeLag(C_BasePlayer* player)
{
	bool rBool = true;
	float CurrentSimtime = player->m_flSimulationTime();
	int i = player->EntIndex();

	if (Simtimes[i] != CurrentSimtime)
		rBool = false;

	Simtimes[i] = CurrentSimtime;
	return rBool;
}

void CRageBot::AutoStop(CUserCmd* cmd)
{
	cmd->forwardmove = 0;
	cmd->sidemove = 0;
	cmd->upmove = 0;
}

void CRageBot::AutoCrouch(CUserCmd* cmd)
{
	cmd->buttons |= IN_DUCK;
}

void CRageBot::ZeusBot(CUserCmd* cmd, C_BaseCombatWeapon* weapon, bool &bSendPacket)
{
	if (!Vars.ragebot_autozeus)
		return;

	//cmd->buttons &= ~IN_ATTACK;

	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
		return;

	C_BasePlayer* local = Globals::LocalPlayer;

	float range = weapon->GetCSWeaponData()->flRange;

	Vector BestPos;
	float BestDmg = 0.f;
	bool Found = false;

	for (int i = 0; i <= Interfaces::GlobalVars->maxClients; i++)
	{
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity || !Globals::LocalPlayer || !entity->IsPlayer() || entity == Globals::LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || entity->IsTeammate())
			continue;

		float Distance = Math::VectorDistance(Globals::LocalPlayer->GetEyePos(), entity->GetEyePos());

		if (Distance > range)
			continue;

		for (int hitboxn = 0; hitboxn < 2; hitboxn++)
		{
			Vector pos = Vector(0, 0, 0);

			switch (hitboxn)
			{
			case 0: //pelvis
				pos = entity->GetHitboxPos(HITBOX_PELVIS);
				break;

			case 1: //chest
				pos = entity->GetHitboxPos(HITBOX_CHEST);
				break;
			}

			if (local->GetEyePos().DistTo(pos) >= range)
				continue;

			float Dmg = CAutoWall::Get().CanHit(pos);

			if (Dmg >= entity->m_iHealth())
			{
				BestDmg = Dmg;
				Found = true;
				BestPos = pos;
			}
		}
	}

	if (!Found)
		return;

	QAngle newAng = Math::CalcAngle(Globals::LocalPlayer->GetEyePos(), BestPos);
	Math::NormalizeAngles(newAng);
	Math::ClampAngles(newAng);

	cmd->viewangles = newAng;
	cmd->buttons |= IN_ATTACK;
	bSendPacket = true;
}

int CRageBot::FindBestEntity(CUserCmd* cmd, C_BaseCombatWeapon* weapon, Vector& hitpos, bool& bBacktrack)
{
	int BestEntityIndex = -1;
	float WeaponRange = weapon->GetCSWeaponData()->flRange;
	float minFoV = (float)Vars.ragebot_fov;
	
	Vector ViewOffset = Globals::LocalPlayer->m_vecOrigin() + Globals::LocalPlayer->m_vecViewOffset();
	QAngle view; Interfaces::Engine->GetViewAngles(view);
	float BestDamage = 0.f;
	Vector BestHitpoint;
	bool BestBacktrack = false;

	for (int i = 0; i <= Interfaces::Engine->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*> (I::EntityList->GetClientEntity(i));
		if (!entity) continue;
		if (!Globals::LocalPlayer) continue;
		if (!entity->IsPlayer()) continue;
		if (!entity->IsAlive()) continue;
		if (entity->IsDormant()) continue;
		if (entity->IsTeammate()) continue;
		if (entity->m_bGunGameImmunity()) continue;

		if (!matrixData[i].gotMatrix)
			continue;

		float Distance = Math::VectorDistance(Globals::LocalPlayer->GetEyePos(), entity->GetEyePos());

		if (Distance > WeaponRange)
			continue;

		float CDamage = 0.f;
		Vector CHitpos;
		bool CUsingBacktrack = false;
		
		bool WillKillEntity = false;

		if (!GetBestHitboxPoint(entity, CDamage, CHitpos, (BaimMode)0, WillKillEntity))
			continue;

		QAngle viewAngles;
		Interfaces::Engine->GetViewAngles(viewAngles);

		float fov = Math::GetFOV(viewAngles, Math::CalcAngle(Globals::LocalPlayer->GetEyePos(), CHitpos));

		if (fov > Vars.ragebot_fov)
			continue;

		switch (Vars.ragebot_selection[curGroup])
		{
		case 0:
			if (fov < minFoV)
			{
				minFoV = fov;
				BestHitpoint = CHitpos;
				BestEntityIndex = entity->EntIndex();
			}
			break;
		case 1:
			if (CDamage > BestDamage)
			{
				BestDamage = CDamage;
				BestHitpoint = CHitpos;
				BestEntityIndex = entity->EntIndex();
			}
			break;
		}
	}

	hitpos = BestHitpoint;
	return BestEntityIndex;
}

bool CRageBot::GetBestHitboxPoint(C_BasePlayer* entity, float& damage, Vector& hitbox, BaimMode baim, bool& WillKill, matrix3x4_t matrix[MAXSTUDIOBONES], mstudiohitboxset_t* StudioSet, bool NoPointscale)
{
	matrix3x4_t cmatrix[MAXSTUDIOBONES];

	if (!matrix)
	{
		matrix = matrixData[entity->EntIndex()].matrix;
	}

	if (!StudioSet)
		StudioSet = matrixData[entity->EntIndex()].StudioSet;

	WillKill = false;

	float BestDamage = 0.f;
	Vector BestHitpoint;
	bool FoundHitableEntity = false;

	CanHitStruct CanHitHead;
	CanHitStruct CanBaimKill;

	for (int hitbox = 0; hitbox < Hitboxes::HITBOX_MAX; hitbox++)
	{
		if ((hitbox == HITBOX_HEAD || hitbox == HITBOX_NECK || hitbox >= HITBOX_RIGHT_THIGH) && baim == BaimMode::FORCE_BAIM)
			continue;

		float pointscale = 0.f;

		switch (hitbox)
		{
		case HITBOX_HEAD:
			if (!Vars.ragebot_hitbox[0][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[0][curGroup];
			break;

		case HITBOX_NECK:
			if (!Vars.ragebot_hitbox[1][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[1][curGroup];
			break;
		case HITBOX_PELVIS:
			if (!Vars.ragebot_hitbox[2][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[2][curGroup];
			break;
		case HITBOX_STOMACH:
			if (!Vars.ragebot_hitbox[3][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[3][curGroup];
			break;
		case HITBOX_LOWER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			if (!Vars.ragebot_hitbox[4][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[4][curGroup];
			break;
		case HITBOX_RIGHT_THIGH:
		case HITBOX_LEFT_THIGH:
		case HITBOX_RIGHT_CALF:
		case HITBOX_LEFT_CALF:
			if (!Vars.ragebot_hitbox[5][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[5][curGroup];
			break;

		case HITBOX_RIGHT_FOOT:
		case HITBOX_LEFT_FOOT:
			if (!Vars.ragebot_hitbox[6][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[6][curGroup];
			break;
		case HITBOX_RIGHT_HAND:
		case HITBOX_LEFT_HAND:
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
			if (!Vars.ragebot_hitbox[7][curGroup])
				continue;
			pointscale = Vars.ragebot_hitbox_multipoint_scale[7][curGroup];
			break;
		default:
			continue;
		}

		if (NoPointscale)
			pointscale = 0.f;

		std::vector<PointScanStruct> Points = GetPointsForScan(entity, hitbox, StudioSet, matrix, pointscale);

		float CBestDamage = 0.f;
		Vector CBestPoint;
		Vector CCenter;
		bool CCanHitCenter = false;
		float CCenterDamage = -1.f;

		for (size_t p = 0; p < Points.size(); p++)
		{
			float CDamage = 0.f;
			CDamage = CAutoWall::Get().CanHit(Points[p].pos);

			if ((hitbox == HITBOX_HEAD || hitbox == HITBOX_NECK) && baim == BaimMode::BAIM)
			{
				if (CDamage >= Vars.ragebot_mindamage[curGroup] && CDamage > CanHitHead.damage)
				{
					CanHitHead.CanHit = true;
					CanHitHead.damage = CDamage;
					CanHitHead.pos = Points[p].pos;
				}

				continue;
			}

			if (baim == BaimMode::BAIM && (hitbox >= HITBOX_RIGHT_HAND || hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH || hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF))
				continue;

			if (Points[p].center && CDamage >= Vars.ragebot_mindamage[curGroup])
			{
				CCanHitCenter = true;
				CCenter = Points[p].pos;
				CCenterDamage = CDamage;
			}


			if (CDamage >= Vars.ragebot_mindamage[curGroup] && CDamage > CBestDamage)
			{
				CBestDamage = CDamage;
				CBestPoint = Points[p].pos;
			}
		}

		if (CCanHitCenter && CCenterDamage >= entity->m_iHealth() / 2.f)
		{
			CBestDamage = CCenterDamage;
			CBestPoint = CCenter;
		}

		if (CBestDamage >= Vars.ragebot_mindamage[curGroup] && CanBaimKill.damage < CBestDamage && (hitbox == HITBOX_CHEST || hitbox == HITBOX_LOWER_CHEST || hitbox == HITBOX_PELVIS || hitbox == HITBOX_STOMACH))
		{
			CanBaimKill.CanHit = true;
			CanBaimKill.damage = CBestDamage;
			CanBaimKill.pos = CBestPoint;
		}


		if (CBestDamage >= Vars.ragebot_mindamage[curGroup] && CBestDamage > BestDamage)
		{
			BestDamage = CBestDamage;
			BestHitpoint = CBestPoint;
			FoundHitableEntity = true;
		}

	}

	if (!FoundHitableEntity && CanHitHead.CanHit)
	{
		FoundHitableEntity = true;
		BestDamage = CanHitHead.damage;
		BestHitpoint = CanHitHead.pos;
	}

	if (CanBaimKill.damage >= entity->m_iHealth())
	{
		FoundHitableEntity = true;
		BestDamage = CanBaimKill.damage;
		BestHitpoint = CanBaimKill.pos;
	}

	if (BestDamage >= entity->m_iHealth())
		WillKill = true;

	damage = BestDamage;
	hitbox = BestHitpoint;

	return FoundHitableEntity;
}

void CRageBot::PVSFix(ClientFrameStage_t stage)
{
	if (stage != FRAME_RENDER_START)
		return;

	for (int i = 1; i <= Interfaces::GlobalVars->maxClients; i++)
	{
		if (i == Interfaces::Engine->GetLocalPlayer()) continue;

		IClientEntity* pCurEntity = Interfaces::EntityList->GetClientEntity(i);
		if (!pCurEntity) continue;

		*(int*)((uintptr_t)pCurEntity + 0xA30) = Interfaces::GlobalVars->framecount; //we'll skip occlusion checks now
		*(int*)((uintptr_t)pCurEntity + 0xA28) = 0;//clear occlusion flags
	}
}

void CRageBot::PrecacheShit()
{
	if (!Vars.ragebot_enabled)
		return;

	for (int i = 0; i <= Interfaces::Engine->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));
		if (!entity) continue;
		if (!Globals::LocalPlayer) continue;
		if (!entity->IsPlayer()) continue;
		if (!entity->IsAlive()) continue;
		if (entity->IsDormant()) continue;
		if (entity->IsTeammate()) continue;

		model_t* pModel = entity->GetModel();

		if (!pModel)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].StudioHdr = g_MdlInfo->GetStudiomodel(pModel);

		if (!matrixData[i].StudioHdr)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].StudioSet = matrixData[i].StudioHdr->GetHitboxSet(0);

		if (!matrixData[i].StudioSet)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].gotMatrix = entity->SetupBones(matrixData[i].matrix, 128, 256, entity->m_flSimulationTime());
	}
}

std::vector<PointScanStruct> CRageBot::GetPointsForScan(C_BasePlayer* entity, int hitbox, mstudiohitboxset_t* hitset, matrix3x4_t matrix[MAXSTUDIOBONES], float pointscale)
{
	std::vector<PointScanStruct> pointsToScan;

	if (!matrix)
		return pointsToScan;

	if (!hitset)
		return pointsToScan;

	mstudiobbox_t* bbox = hitset->GetHitbox(hitbox);

	if (!bbox)
		return pointsToScan;

	float mod = bbox->m_flRadius != -1.f ? bbox->m_flRadius : 0.f;

	Vector max;
	Vector min;

	Vector in1 = bbox->bbmax + mod;
	Vector in2 = bbox->bbmin - mod;


	Math::VectorTransform(in1, matrix[bbox->bone], max);
	Math::VectorTransform(in2, matrix[bbox->bone], min);

	Vector center = (min + max) * 0.5f;

	QAngle curAngles = Math::CalcAngle(center, Globals::LocalPlayer->GetEyePos());

	Vector forward;
	Math::AngleVectors(curAngles, forward);

	Vector right = forward.Cross(Vector(0, 0, 1));
	Vector left = Vector(-right.x, -right.y, right.z);

	Vector top = Vector(0, 0, 1);
	Vector bot = Vector(0, 0, -1);

	if (pointscale == 0.f)
	{
		pointsToScan.emplace_back(PointScanStruct{ center, true });
		return pointsToScan;
	}

	if (hitbox == HITBOX_HEAD)
	{
		for (auto i = 0; i < 5; ++i)
		{
			pointsToScan.emplace_back(PointScanStruct{ center });
		}

		pointsToScan[1].pos += top * (bbox->m_flRadius * pointscale);
		pointsToScan[2].pos += right * (bbox->m_flRadius * pointscale);
		pointsToScan[3].pos += left * (bbox->m_flRadius * pointscale);
		pointsToScan[4].pos = center;
		pointsToScan[4].center = true;
	}
	else
	{
		for (auto i = 0; i < 3; ++i)
		{
			pointsToScan.emplace_back(PointScanStruct{ center });
		}

		pointsToScan[0].pos += right * (bbox->m_flRadius * pointscale);
		pointsToScan[1].pos += left * (bbox->m_flRadius * pointscale);
		pointsToScan[2].pos = center;
		pointsToScan[2].center = true;
	}

	return pointsToScan;
}
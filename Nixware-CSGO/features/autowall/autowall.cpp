#include "autowall.hpp"
#include "../../helpers/math.hpp"
#include "../../options.hpp"

#define HITGROUP_GENERIC   0
#define HITGROUP_HEAD      1
#define HITGROUP_CHEST     2
#define HITGROUP_STOMACH   3
#define HITGROUP_LEFTARM   4
#define HITGROUP_RIGHTARM  5
#define HITGROUP_LEFTLEG   6
#define HITGROUP_RIGHTLEG  7
#define HITGROUP_GEAR      10

inline bool CGameTrace::DidHitWorld() const
{
	return hit_entity->EntIndex() == 0;
}
inline bool CGameTrace::DidHitNonWorldEntity() const
{
	return hit_entity != NULL && !DidHitWorld();
}

#define HITGROUP_GENERIC    0
#define HITGROUP_HEAD        1
#define HITGROUP_CHEST        2
#define HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10
#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1
#define DAMAGE_YES		2
#define DAMAGE_AIM		3
#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< wierd-looking jello effect for advisor shield.

void ScaleDamage_1(int hitgroup, C_BasePlayer* enemy, float weapon_armor_ratio, float& current_damage)
{
	int armor = enemy->m_ArmorValue();
	float ratio;

	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		current_damage *= 4.f;
		break;
	case HITGROUP_STOMACH:
		current_damage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage *= 0.75f;
		break;
	}

	if (armor > 0)
	{
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			if (enemy->m_bHasHelmet())
			{
				ratio = (weapon_armor_ratio * 0.5) * current_damage;
				if (((current_damage - ratio) * 0.5) > armor)
				{
					ratio = current_damage - (armor * 2.0);
				}
				current_damage = ratio;
			}
			break;
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			ratio = (weapon_armor_ratio * 0.5) * current_damage;
			if (((current_damage - ratio) * 0.5) > armor)
			{
				ratio = current_damage - (armor * 2.0);
			}
			current_damage = ratio;
			break;
		}
	}
}

void CAutoWall::TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	Interfaces::EngineTrace->TraceRay(ray, mask, &filter, ptr);
}

void CAutoWall::ClipTraceToPlayers(const Vector& absStart, const Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr)
{
	C_BasePlayer* pLocal = Globals::LocalPlayer;// Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon* weapon = pLocal->m_hActiveWeapon();//(C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	static DWORD dwAddress = (DWORD)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");

	if (!dwAddress)
	{
		return;
	}

	if (weapon->IsGrenade() || weapon->IsKnife())
	{
		return;
	}

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, absEnd
		LEA		ECX, absStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}

////////////////////////////////////// Legacy Functions //////////////////////////////////////
void CAutoWall::GetBulletTypeParameters(float& maxRange, float& maxDistance, char* bulletType, bool sv_penetration_type)
{
	if (sv_penetration_type)
	{
		maxRange = 35.0;
		maxDistance = 3000.0;
	}
	else
	{
		//Play tribune to framerate. Thanks, stringcompare
		//Regardless I doubt anyone will use the old penetration system anyway; so it won't matter much.
		if (!strcmp(bulletType, ("BULLET_PLAYER_338MAG")))
		{
			maxRange = 45.0;
			maxDistance = 8000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_762MM")))
		{
			maxRange = 39.0;
			maxDistance = 5000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_556MM")) || !strcmp(bulletType, ("BULLET_PLAYER_556MM_SMALL")) || !strcmp(bulletType, ("BULLET_PLAYER_556MM_BOX")))
		{
			maxRange = 35.0;
			maxDistance = 4000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_57MM")))
		{
			maxRange = 30.0;
			maxDistance = 2000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_50AE")))
		{
			maxRange = 30.0;
			maxDistance = 1000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_357SIG")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_SMALL")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_P250")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_MIN")))
		{
			maxRange = 25.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_9MM")))
		{
			maxRange = 21.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_45ACP")))
		{
			maxRange = 15.0;
			maxDistance = 500.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_BUCKSHOT")))
		{
			maxRange = 0.0;
			maxDistance = 0.0;
		}
	}
}

////////////////////////////////////// Misc Functions //////////////////////////////////////
bool CAutoWall::BreakableEntity(IClientEntity* entity)
{

	ClientClass* pClass = (ClientClass*)entity->GetClientClass();

	if (!pClass)
	{
		return false;
	}

	if (pClass == nullptr)
	{
		return false;
	}

	return pClass->m_ClassID == ClassId_t::CBreakableProp || pClass->m_ClassID == ClassId_t::CBreakableSurface;

}

void CAutoWall::ScaleDamage(CGameTrace& enterTrace, CCSWeaponInfo* weaponData, float& currentDamage)
{

	C_BasePlayer* pLocal = Globals::LocalPlayer;//Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon* weapon = pLocal->m_hActiveWeapon();//(C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	bool hasHeavyArmor = false;
	int armorValue = ((C_BasePlayer*)enterTrace.hit_entity)->m_ArmorValue();
	int hitGroup = enterTrace.hitgroup;

	if (!pLocal)
	{
		return;
	}

	if (weapon->IsZeus() || weapon->IsGrenade() || weapon->IsKnife())
	{
		return;
	}

	auto IsArmored = [&enterTrace]()->bool
	{
		C_BasePlayer* targetEntity = (C_BasePlayer*)enterTrace.hit_entity;
		switch (enterTrace.hitgroup)
		{
		case HITGROUP_HEAD:
			return !!targetEntity->m_bHasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hitGroup)
	{
	case HITGROUP_HEAD:
		currentDamage *= 2.f;
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= 0.75f;
		break;
	default:
		break;
	}

	if (armorValue > 0 && IsArmored())
	{
		float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = weaponData->flArmorRatio / 2.f;

		if (hasHeavyArmor)
		{
			armorBonusRatio = 0.33f;
			armorRatio *= 0.5f;
			bonusValue = 0.33f;
		}

		auto NewDamage = currentDamage * armorRatio;

		if (((currentDamage - (currentDamage * armorRatio)) * (bonusValue * armorBonusRatio)) > armorValue)
		{
			NewDamage = currentDamage - (armorValue / armorBonusRatio);
		}

		currentDamage = NewDamage;
	}
}

////////////////////////////////////// Main Autowall Functions //////////////////////////////////////
bool CAutoWall::trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction)
{
	Vector start, end;
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while (currentDistance <= maxDistance)
	{
		currentDistance += rayExtension;

		start = startPosition + direction * currentDistance;

		if (!firstContents)
		{
			firstContents = Interfaces::EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);
		}

		int pointContents = Interfaces::EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!(pointContents & MASK_SHOT_HULL) || pointContents & CONTENTS_HITBOX && pointContents != firstContents)
		{
			end = start - (direction * rayExtension);

			TraceLine(start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace);

			if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
			{
				TraceLine(start, startPosition, MASK_SHOT_HULL, exitTrace.hit_entity, &exitTrace);

				if (exitTrace.DidHit() && !exitTrace.startsolid)
				{
					start = exitTrace.endpos;
					return true;
				}
				continue;
			}

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{

				if (BreakableEntity(enterTrace.hit_entity) && BreakableEntity(exitTrace.hit_entity))
				{
					return true;
				}

				if (enterTrace.surface.flags & SURF_NODRAW || !(exitTrace.surface.flags & SURF_NODRAW) && (exitTrace.plane.normal.Dot(direction) <= 1.f))
				{
					float multAmount = exitTrace.fraction * 4.f;
					start -= direction * multAmount;
					return true;
				}

				continue;
			}

			if (!exitTrace.DidHit() || exitTrace.startsolid)
			{
				if (enterTrace.DidHitNonWorldEntity() && BreakableEntity(enterTrace.hit_entity))
				{
					//auto t = enterTrace;
					exitTrace = enterTrace;
					exitTrace.endpos = start + direction;
					return true;
				}

				continue;
			}
		}
	}
	return false;
}

bool CAutoWall::HandleBulletPenetration(CCSWeaponInfo* weaponData, CGameTrace& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration)
{
	//Because there's been issues regarding this- putting this here.
	if (&currentDamage == nullptr)
	{
		handle_penetration = false;
		return false;
	}

	C_BasePlayer* local = Globals::LocalPlayer;//(IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	FireBulletData data(local->GetEyePos());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	CGameTrace exitTrace;
	C_BasePlayer* pEnemy = (C_BasePlayer*)enterTrace.hit_entity;
	surfacedata_t* enterSurfaceData = Interfaces::PhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);
	int enterMaterial = enterSurfaceData->game.material;

	float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
	float enterDamageModifier = enterSurfaceData->game.flDamageModifier;
	float thickness, modifier, lostDamage, finalDamageModifier, combinedPenetrationModifier;
	bool isSolidSurf = ((enterTrace.contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((enterTrace.surface.flags >> 7) & SURF_LIGHT);

	if (possibleHitsRemaining <= 0
		|| (enterTrace.surface.name == (const char*)0x2227c261 && exitTrace.surface.name == (const char*)0x2227c868)
		|| (!possibleHitsRemaining && !isLightSurf && !isSolidSurf && enterMaterial != CHAR_TEX_GRATE && enterMaterial != CHAR_TEX_GLASS)
		|| weaponData->flPenetration <= 0.f
		|| !trace_to_exit(enterTrace, exitTrace, enterTrace.endpos, direction)
		&& !(Interfaces::EngineTrace->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL, nullptr) & MASK_SHOT_HULL))
	{
		handle_penetration = false;
		return false;
	}

	surfacedata_t* exitSurfaceData = Interfaces::PhysSurface->GetSurfaceData(exitTrace.surface.surfaceProps);
	int exitMaterial = exitSurfaceData->game.material;
	float exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;
	float exitDamageModifier = exitSurfaceData->game.flDamageModifier;

	if (sv_penetration_type)
	{
		if (enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS)
		{
			combinedPenetrationModifier = 3.f;
			finalDamageModifier = 0.05f;
		}
		else if (isSolidSurf || isLightSurf)
		{
			combinedPenetrationModifier = 1.f;
			finalDamageModifier = 0.16f;
		}
		else if (enterMaterial == CHAR_TEX_FLESH && (pEnemy->IsTeammate() && ff_damage_reduction_bullets == 0.f))
		{
			if (ff_damage_bullet_penetration == 0.f)
			{
				handle_penetration = false;
				return false;
			}
			combinedPenetrationModifier = ff_damage_bullet_penetration;
			finalDamageModifier = 0.16f;
		}
		else
		{
			combinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) / 2.f;
			finalDamageModifier = 0.16f;
		}

		if (enterMaterial == exitMaterial)
		{
			if (exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD)
			{
				combinedPenetrationModifier = 3.f;
			}
			else if (exitMaterial == CHAR_TEX_PLASTIC)
			{
				combinedPenetrationModifier = 2.f;
			}
		}

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();
		modifier = fmaxf(1.f / combinedPenetrationModifier, 0.f);

		lostDamage = fmaxf(
			((modifier * thickness) / 24.f)
			+ ((currentDamage * finalDamageModifier)
				+ (fmaxf(3.75f / penetrationPower, 0.f) * 3.f * modifier)), 0.f);

		if (lostDamage > currentDamage)
		{
			handle_penetration = false;
			return false;
		}

		if (lostDamage > 0.f)
		{
			currentDamage -= lostDamage;
		}

		if (currentDamage < 1.f)
		{
			handle_penetration = false;
			return false;
		}

		eyePosition = exitTrace.endpos;
		--possibleHitsRemaining;

		handle_penetration = true;
		return true;
	}
	else
	{
		combinedPenetrationModifier = 1.f;

		if (isSolidSurf || isLightSurf)
		{
			finalDamageModifier = 0.99f;
		}
		else
		{
			finalDamageModifier = fminf(enterDamageModifier, exitDamageModifier);
			combinedPenetrationModifier = fminf(enterSurfPenetrationModifier, exitSurfPenetrationModifier);
		}

		if (enterMaterial == exitMaterial && (exitMaterial == CHAR_TEX_METAL || exitMaterial == CHAR_TEX_WOOD))
		{
			combinedPenetrationModifier += combinedPenetrationModifier;
		}

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();

		if (sqrt(thickness) <= combinedPenetrationModifier * penetrationPower)
		{
			currentDamage *= finalDamageModifier;
			eyePosition = exitTrace.endpos;
			--possibleHitsRemaining;
			handle_penetration = true;
			return true;
		}
		handle_penetration = false;
		return false;
	}
}

bool CAutoWall::FireBullet(C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage)
{
	if (!pWeapon)
	{
		return false;
	}

	C_BasePlayer* local = Globals::LocalPlayer;//(IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	//FireBulletData data(local->GetEyePos());

	//data.filter = CTraceFilter();
	//data.filter.pSkip = local;

	bool sv_penetration_type;

	float currentDistance = 0.f, penetrationPower, penetrationDistance, maxRange, ff_damage_reduction_bullets, ff_damage_bullet_penetration, rayExtension = 40.f;
	Vector eyePosition = local->GetEyePos();

	static ConVar* penetrationSystem = Interfaces::Convar->FindVar(("sv_penetration_type"));
	static ConVar* damageReductionBullets = Interfaces::Convar->FindVar(("ff_damage_reduction_bullets"));
	static ConVar* damageBulletPenetration = Interfaces::Convar->FindVar(("ff_damage_bullet_penetration"));

	sv_penetration_type = penetrationSystem->GetBool();
	ff_damage_reduction_bullets = damageReductionBullets->GetFloat();
	ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	CCSWeaponInfo* weaponData = pWeapon->GetCSWeaponData();
	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = local;

	if (!weaponData)
	{
		return false;
	}

	maxRange = weaponData->flRange;

	GetBulletTypeParameters(penetrationPower, penetrationDistance, weaponData->szBulletType, sv_penetration_type);

	if (sv_penetration_type)
	{
		penetrationPower = weaponData->flPenetration;
	}

	int possibleHitsRemaining = 4;

	currentDamage = weaponData->iDamage;

	while (possibleHitsRemaining > 0 && currentDamage >= 1.f)
	{
		maxRange -= currentDistance;

		Vector end = eyePosition + direction * maxRange;

		TraceLine(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, local, &enterTrace);
		ClipTraceToPlayers(eyePosition, end + direction * rayExtension, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace); //  | CONTENTS_HITBOX

		surfacedata_t* enterSurfaceData = Interfaces::PhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);

		float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;

		int enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.f)
		{
			break;
		}

		currentDistance += enterTrace.fraction * maxRange;

		currentDamage *= pow(weaponData->flRangeModifier, (currentDistance / 500.f));

		if (currentDistance > penetrationDistance && weaponData->flPenetration > 0.f || enterSurfPenetrationModifier < 0.1f)
		{
			break;
		}

		bool canDoDamage = (enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC);

		if (canDoDamage && !static_cast<C_BasePlayer*>(enterTrace.hit_entity)->IsTeammate())
		{
			ScaleDamage(enterTrace, weaponData, currentDamage);
			return true;
		}

		if (!HandleBulletPenetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, sv_penetration_type, ff_damage_reduction_bullets, ff_damage_bullet_penetration))
		{
			break;
		}
	}
	return false;
}

////////////////////////////////////// Usage Calls //////////////////////////////////////
float CAutoWall::CanHit(Vector& point)
{
	C_BasePlayer* local = Globals::LocalPlayer;//(IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (!local || !local->IsAlive())
	{
		return -1.f;
	}

	FireBulletData data(local->GetEyePos());// = FireBulletData(local->GetEyePosition());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	Vector angles, direction;
	Vector tmp = point - local->GetEyePos();
	float currentDamage = 0;

	//VectorAngles(tmp, angles);
	//AngleVectors(angles, &direction);
	direction = tmp;
	direction.NormalizeInPlace();

	if (FireBullet(local->m_hActiveWeapon(), direction, currentDamage))
	{
		return currentDamage;
	}
	return -1.f;
}

float CAutoWall::CanHit(Vector& start, Vector& point)
{
	C_BasePlayer* local = Globals::LocalPlayer;

	if (!local || !local->IsAlive())
	{
		return -1.f;
	}

	FireBulletData data(start);
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	Vector angles, direction;
	Vector tmp = point - start;
	float currentDamage = 0;

	direction = tmp;
	direction.NormalizeInPlace();

	if (FireBullet(local->m_hActiveWeapon(), direction, currentDamage))
	{
		return currentDamage;
	}
	return -1.f;
}

float CAutoWall::CanHit(C_BasePlayer* ent, Vector& point)
{
	if (!ent || !ent->IsAlive())
	{
		return -1.f;
	}

	FireBulletData data(ent->GetEyePos());
	data.filter = CTraceFilter();
	data.filter.pSkip = ent;
	Vector angles, direction;
	Vector tmp = point - ent->GetEyePos();
	float currentDamage = 0;
	direction = tmp;
	direction.NormalizeInPlace();

	if (FireBullet(ent->m_hActiveWeapon(), direction, currentDamage))
	{
		return currentDamage;
	}
	return -1.f;
}

bool CAutoWall::trace_awall(float& damage)
{
	C_BasePlayer* local = Globals::LocalPlayer;
	if (!local)
	{
		return false;
	}
	FireBulletData data(local->GetEyePos());
	Vector eyepos = local->GetEyePos();
	data.filter = CTraceFilter();
	data.filter.pSkip = local;

	QAngle EyeAng = QAngle(0, 0, 0);
	Interfaces::Engine->GetViewAngles(EyeAng);

	Vector dst, forward;

	Math::AngleVectors(EyeAng, forward);
	dst = data.src + (forward * 8196.f);

	QAngle angles = QAngle(0, 0, 0);
	angles = Math::CalcAngle(data.src, dst);
	Math::AngleVectors(angles, data.direction);
	Math::Normalize3(data.direction);

	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)local->m_hActiveWeapon().Get();

	if (!weapon)
	{
		return false;
	}

	data.penetrate_count = 1;
	data.trace_length = 0.0f;

	CCSWeaponInfo* weaponData = weapon->GetCSWeaponData();

	if (!weaponData)
	{
		return false;
	}

	data.current_damage = (float)weaponData->iDamage;

	data.trace_length_remaining = weaponData->flRange - data.trace_length;

	Vector end = data.src + data.direction * data.trace_length_remaining;

	TraceLine(data.src, end, MASK_SHOT | CONTENTS_GRATE, local, &data.enter_trace);

	if (data.enter_trace.fraction == 1.0f)
	{
		return false;
	}

	if (FireBullet(weapon, data.direction, damage))
	{
		return true;
	}

	return false;
}

bool CAutoWall::trace_awall(C_BasePlayer* m_local, Vector hit, float& damage)
{
	if (m_local && m_local->IsAlive())
	{
		FireBulletData data(m_local->GetEyePos());
		data.filter = CTraceFilter();
		data.filter.pSkip = m_local;
		QAngle angles;
		angles = Math::CalcAngle(data.src, hit);
		angles.Normalize();
		Math::AngleVectors(angles, data.direction);
		auto m_weapon = m_local->m_hActiveWeapon();
		if (m_weapon)
		{
			data.penetrate_count = 1;
			data.trace_length = 0.0f;
			CCSWeaponInfo* weapon_data = m_weapon->GetCSWeaponData();
			if (weapon_data)
			{
				data.current_damage = weapon_data->iDamage;
				data.trace_length_remaining = weapon_data->flRange - data.trace_length;
				Vector end = data.src + data.direction * data.trace_length_remaining;
				TraceLine(data.src, end, MASK_SHOT | CONTENTS_GRATE, m_local, &data.enter_trace);
				if (data.enter_trace.fraction == 1.0f)
				{
					return false;
				}
				if (handle_penetration)
				{
					damage = data.current_damage;
					return true;
				}
			}
		}
	}
	return false;
}

#include "Resolver.hpp"
#include "../autowall/autowall.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include "../../helpers\math.hpp"

/*
is fakelag --> set first tick angle
*/

bool InLc = false;
bool InLbyUpdate = false;
bool Moving = false;

void Resolver::OnCreateMove(QAngle OrgViewang)
{
	return;
}

void Resolver::OnFramestageNotify()
{
	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity || !Globals::LocalPlayer || !entity->IsPlayer() || entity == Globals::LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || entity->IsTeammate())
		{
			g_Resolver.StandAAData[i].UsedEdge = false;
			SavedResolverData[i].WasDormantBetweenMoving = true;
			SavedResolverData[i].CanUseMoveStandDelta = false;
			continue;
		}

		bool InFakeLag = true;

		if (SavedResolverData[i].LastSimtime != entity->m_flSimulationTime())
			InFakeLag = false;

		SavedResolverData[i].LastPos = entity->m_vecOrigin();

		if (InFakeLag)
			g_Resolver.GResolverData[i].BreakingLC = (entity->m_vecOrigin() - SavedResolverData[i].LastPos).LengthSqr() > 4096.f;

		SavedResolverData[i].LastSimtime = entity->m_flSimulationTime();

		//SavedResolverData[i].UsingAA

		//entity->SetAbsOriginal(entity->m_vecOrigin()); //xD

		//entity->SetAbsOriginal(entity->m_vecOrigin()); //fix possition

		g_Resolver.GResolverData[i].InFakelag = InFakeLag;

		ResolverAnimDataStorage AnimationStorage;
		AnimationStorage.UpdateAnimationLayers(entity);

		bool Moving = entity->m_vecVelocity().Length2D() > 0.1f;
		bool SlowWalk = Moving && entity->m_vecVelocity().Length2D() < 52.f && fabs(SavedResolverData[i].LastVel - entity->m_vecVelocity().Length2D()) < 4.f;
		bool InAir = !(entity->m_fFlags() & FL_ONGROUND);
		bool Standing = !Moving && !InAir;

		SavedResolverData[i].LastVel = entity->m_vecVelocity().Length2D();

		if (!InFakeLag)
			AddCurrentYaw(entity, i);

		float avgang = 0.f;
		bool b = GetAverageYaw(i, avgang);

		if (Moving && !SlowWalk && !InAir)
			g_Resolver.GResolverData[i].Resolved = true;
		else if (Moving && SlowWalk && !InAir)
		{
			if (IsStaticYaw(i) && b)
			{
				if (g_Resolver.GResolverData[i].Shots >= 1)
				{
					switch (g_Resolver.GResolverData[i].Shots % 4)
					{
					case 0:
						entity->m_angEyeAngles().yaw += 58.f;
						break;

					case 1:
						entity->m_angEyeAngles().yaw -= 58.f;
						break;

					case 2:
						entity->m_angEyeAngles().yaw += 29.f;
						break;

					case 3:
						entity->m_angEyeAngles().yaw -= 29.f;
						break;
					}
				}
			}

			g_Resolver.GResolverData[i].Resolved = false;

		}
		else if (InAir)
			g_Resolver.GResolverData[i].Resolved = true;
		else
		{
			g_Resolver.GResolverData[i].Resolved = true;
			float fl_ang = fabs(fabs(entity->m_flLowerBodyYawTarget()) - fabs(entity->m_angEyeAngles().yaw));
			bool fake = (fl_ang >= 45.f && fl_ang <= 85.f) || !IsStaticYaw(i);
			float Yaw = entity->m_angEyeAngles().yaw;

			if (fake)
			{
				g_Resolver.GResolverData[i].Resolved = false;

				if (!IsStaticYaw(i) && b)
					Yaw = avgang;
			}

			if (fake && g_Resolver.GResolverData[i].Shots >= 1)
			{
				switch (g_Resolver.GResolverData[i].Shots % 4)
				{
				case 0:
					entity->m_angEyeAngles().yaw = Yaw + 58.f;
					break;

				case 1:
					entity->m_angEyeAngles().yaw = Yaw - 58.f;
					break;

				case 2:
					entity->m_angEyeAngles().yaw = Yaw + 29;
					break;

				case 3:
					entity->m_angEyeAngles().yaw = Yaw - 29;
					break;
				}
			}
		}

		g_Resolver.GResolverData[i].Fake = !g_Resolver.GResolverData[i].Resolved;
	}
}

void Resolver::OnFireEvent(IGameEvent* event)
{
	if (!Globals::LocalPlayer)
		return;

	static float LastPlayerHurt = 0.f;

	if (!strcmp(event->GetName(), "player_hurt"))
	{
		if (g_GlobalVars->curtime == LastPlayerHurt)
			return;

		LastPlayerHurt = g_GlobalVars->curtime;

		int i = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		//C_BasePlayer* p = g_EntityList->GetClientEntity(i);
		//C_BasePlayer* p = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
		int attacker = g_EngineClient->GetPlayerForUserID(event->GetInt("attacker"));

		if (attacker != Globals::LocalPlayer->EntIndex())
			return;

		C_BasePlayer* user = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!user)
			return;

		std::string hitgroup = "";

		switch (event->GetInt("hitgroup"))
		{
		case HITBOX_HEAD:
			hitgroup = "HEAD";
			break;

		case HITBOX_NECK:
			hitgroup = "NECK";
			break;

		case HITBOX_PELVIS:
			hitgroup = "PELVIS";
			break;

		case HITBOX_STOMACH:
			hitgroup = "STOMACH";
			break;

		case HITBOX_LOWER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			hitgroup = "CHEST";
			break;

		case HITBOX_RIGHT_THIGH:
		case HITBOX_LEFT_THIGH:
			hitgroup = "THIGH";
			break;

		case HITBOX_RIGHT_CALF:
		case HITBOX_LEFT_CALF:
			hitgroup = "CALF";
			break;

		case HITBOX_RIGHT_FOOT:
		case HITBOX_LEFT_FOOT:
			hitgroup = "FOOT";
			break;

		case HITBOX_RIGHT_HAND:
		case HITBOX_LEFT_HAND:
			hitgroup = "HAND";
			break;

		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
			hitgroup = "ARM";
			break;
		}

		//std::string InLbyUpdate = g_Saver.RbotShotInfo.InLbyUpdate ? "TRUE" : "FALSE";

		if (!InLbyUpdate)
		{
			g_Resolver.GResolverData[i].ShotsAtMode[(int)g_Resolver.GResolverData[i].mode]--;
			g_Resolver.GResolverData[i].Shots--;

			if (g_Resolver.GResolverData[i].Shots < 0)
				g_Resolver.GResolverData[i].Shots = 0;
		}
	}

	if (!strcmp(event->GetName(), "player_death"))
	{
		int i = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		g_Resolver.GResolverData[i].Shots = 0;
		SavedResolverData[i].MoveStandDelta = 0.f;

		for (int i2 = 0; i2 < (int)ResolverModes::MAX; i2++)
			g_Resolver.GResolverData[i].ShotsAtMode[i2] = 0;
	}

	static std::string lastName = "";
	static std::string lastWeapon = "";

	if (!strcmp(event->GetName(), "item_purchase"))
	{
		int i = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		C_BasePlayer* user = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!user)
			return;

		if (lastName == user->GetPlayerInfo().szName && lastWeapon == event->GetString("weapon"))
			return;

		lastName = user->GetPlayerInfo().szName;
		lastWeapon = event->GetString("weapon");
	}

	if (!strcmp(event->GetName(), "round_end"))
	{
		for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
		{
			g_Resolver.GResolverData[i].Shots = 0;
			SavedResolverData[i].MoveStandDelta = 0.f;

			for (int i2 = 0; i2 < (int)ResolverModes::MAX; i2++)
				g_Resolver.GResolverData[i].ShotsAtMode[i2] = 0;
		}
	}

	if (!strcmp(event->GetName(), "player_disconnect"))
	{
		int i = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));

		if (i == g_EngineClient->GetLocalPlayer())
			return;

		g_Resolver.GResolverData[i].Shots = 0;
		SavedResolverData[i].MoveStandDelta = 0.f;

		for (int i2 = 0; i2 < (int)ResolverModes::MAX; i2++)
			g_Resolver.GResolverData[i].ShotsAtMode[i2] = 0;
	}
}

#ifdef FakeAnglesEnabled
bool Resolver::InFakeWalk(C_BasePlayer* player, ResolverAnimDataStorage store)
{
	/*
	bool
		bFakewalking = false,
		stage1 = false,			// stages needed cause we are iterating all layers, eitherwise won't work :)
		stage2 = false,
		stage3 = false;

	for (int i = 0; i < store.m_iLayerCount; i++)
	{
		if (store.animationLayer[i].m_nSequence == 26 && store.animationLayer[i].m_flWeight < 0.47f)
			stage1 = true;
		if (store.animationLayer[i].m_nSequence == 7 && store.animationLayer[i].m_flWeight > 0.001f)
			stage2 = true;
		if (store.animationLayer[i].m_nSequence == 2 && store.animationLayer[i].m_flWeight == 0)
			stage3 = true;
	}

	if (stage1 && stage2)
		if (stage3 || (player->m_fFlags() & FL_DUCKING)) // since weight from stage3 can be 0 aswell when crouching, we need this kind of check, cause you can fakewalk while crouching, thats why it's nested under stage1 and stage2
			bFakewalking = true;
		else
			bFakewalking = false;
	else
		bFakewalking = false;

	return bFakewalking;
	*/
	AnimationLayer anim_layers[15];

	bool check_1 = false, check_2 = false, check_3 = false;

	for (int i = 0; i < store.m_iLayerCount; i++)
	{
		anim_layers[i] = store.animationLayer[i];

		if (anim_layers[i].m_nSequence == 8 && anim_layers[i].m_flWeight == 1)
			check_1 = true;

		if (anim_layers[i].m_nSequence == 7 && anim_layers[i].m_flWeight > 0.f && anim_layers[i].m_flWeight < 0.003f)
			check_2 = true;

		if (anim_layers[i].m_nSequence == 26 && anim_layers[i].m_flWeight > 0.f && anim_layers[i].m_flWeight < 0.3f  && anim_layers[i].m_flPlaybackRate > 0.f && anim_layers[i].m_flPlaybackRate < 0.001f)
			check_3 = true;

	}

	if ((check_1 && check_2 && check_3))
		return true;
	else
		return false;

	return false;
}

bool Resolver::InFakeWalkOld(C_BasePlayer* player, ResolverAnimDataStorage store)
{
	bool
		bFakewalking = false,
		stage1 = false,			// stages needed cause we are iterating all layers, eitherwise won't work :)
		stage2 = false,
		stage3 = false;

	for (int i = 0; i < store.m_iLayerCount; i++)
	{
		if (store.animationLayer[i].m_nSequence == 26 && store.animationLayer[i].m_flWeight < 0.47f)
			stage1 = true;

		if (store.animationLayer[i].m_nSequence == 7 && store.animationLayer[i].m_flWeight > 0.001f)
			stage2 = true;

		if (store.animationLayer[i].m_nSequence == 2 && store.animationLayer[i].m_flWeight == 0)
			stage3 = true;
	}

	if (stage1 && stage2)
		if (stage3 || (player->m_fFlags() & FL_DUCKING)) // since weight from stage3 can be 0 aswell when crouching, we need this kind of check, cause you can fakewalk while crouching, thats why it's nested under stage1 and stage2
			bFakewalking = true;
		else
			bFakewalking = false;
	else
		bFakewalking = false;

	return bFakewalking;
}

bool Resolver::IsValidAngle(C_BasePlayer* player, float angle)
{
	return fabs(player->m_flLowerBodyYawTarget() - angle) > 30.f;
}

bool Resolver::GetWallDistance(C_BasePlayer* player, float& val)
{
	if (!player)
		return false;

	Vector position = player->m_vecOrigin() + player->m_vecViewOffset();
	float closest_distance = 250.f;
	float radius = 250.f;
	float step = M_PI * 2.0 / 18;

	for (float a = 0; a < (M_PI * 2.0); a += step)
	{
		Vector location(radius * cos(a) + position.x, radius * sin(a) + position.y, position.z);
		Ray_t ray;
		trace_t tr;
		ray.Init(position, location);
		CTraceFilter traceFilter;
		traceFilter.pSkip = player;
		g_EngineTrace->TraceRay(ray, 0x4600400B, &traceFilter, &tr);

		float distance = position.DistTo(tr.endpos);

		if (distance < closest_distance)
		{
			closest_distance = distance;
			//angle.yaw = RAD2DEG(a);
		}
	}

	if (closest_distance >= radius)
		return false;

	val = closest_distance;
	return true;
}

bool Resolver::Is979(C_BasePlayer* player, ResolverAnimDataStorage store, AnimationLayer& layer)
{
	int layers = store.m_iLayerCount;

	for (int i = 0; i < layers; i++)
	{
		const int activity = player->GetSequenceActivity(store.animationLayer[i].m_nSequence);

		if (activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST)
		{
			layer = store.animationLayer[i];
			return true;
		}
	}

	return false;
}

bool Resolver::FreestandingSim(C_BasePlayer* player, float& ang)
{
	if (!player || !player->IsAlive() || !Globals::LocalPlayer)
		return false;

	bool no_active = true;
	static float hold = 0.f;
	Vector besthead;

	auto leyepos = player->m_vecOrigin() + player->m_vecViewOffset();
	auto headpos = player->GetHitboxPos(0); //GetHitboxPosition(local_player, 0);
	auto origin = player->m_vecOrigin();

	auto checkWallThickness = [&](C_BasePlayer * pPlayer, Vector newhead) -> float
	{

		Vector endpos1, endpos2;

		Vector eyepos = pPlayer->m_vecOrigin() + pPlayer->m_vecViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, player);

		trace_t trace1, trace2;
		g_EngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &trace1);

		if (trace1.DidHit())
			endpos1 = trace1.endpos;
		else
			return 0.f;

		ray.Init(eyepos, newhead);
		g_EngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &trace2);

		if (trace2.DidHit())
			endpos2 = trace2.endpos;

		float add = newhead.DistTo(eyepos) - leyepos.DistTo(eyepos) + 3.f;
		return endpos1.DistTo(endpos2) + add / 3;

	};

	//int index = GetNearestPlayerToCrosshair();
	//static C_BasePlayer* entity;

	if (!player->IsAlive())
		hold = 0.f;


	float radius = Vector(headpos - origin).Length2D();

	auto CheckAllAnglesForEntity = [&](C_BasePlayer * pPlayer, bool & CanUse) -> float
	{
		CanUse = false;

		if (!pPlayer)
			return 0.f;

		float cbestrotation = 0.f;
		float chighestthickness = 0.f;

		float step = (M_PI * 2) / 90;

		for (float besthead = 0; besthead < (M_PI * 2); besthead += step)
		{
			Vector newhead(radius * cos(besthead) + leyepos.x, radius * sin(besthead) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			totalthickness += checkWallThickness(pPlayer, newhead);

			if (totalthickness > chighestthickness && fabs(pPlayer->m_flLowerBodyYawTarget() - RAD2DEG(besthead)) > 20.f
				&& fabs(SavedResolverData[pPlayer->EntIndex()].FakeYaw - RAD2DEG(besthead)) > 20.f)
			{
				chighestthickness = totalthickness;
				cbestrotation = besthead;
				CanUse = true;
			}
		}

		return cbestrotation;
	};


	float r = 0.f;
	int c = 0;

	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
	{
		std::vector<C_BasePlayer*> enemies;

		for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
		{
			auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

			if (!entity || !Globals::LocalPlayer || !entity->IsPlayer() || entity == player || entity->IsDormant()
				|| !entity->IsAlive() || !entity->IsEnemy())
				continue;

			enemies.push_back(entity);
		}

		if (enemies.size() != 0)
			no_active = false;

		for (auto enemy = enemies.begin(); enemy != enemies.end(); enemy++)
		{
			bool b = false;
			float t = RAD2DEG(CheckAllAnglesForEntity(*enemy, b));

			if (b)
			{
				c++;
				r += t;
			}
		}
	}
	else
	{
		c = 1;
		bool b = false;
		r = RAD2DEG(CheckAllAnglesForEntity(Globals::LocalPlayer, b));

		if (!b)
			return false;

		no_active = false;
	}

	if (no_active)
		return false;
	else
	{
		ang = r / c;
		return true;
	}

	return false;
}

bool Resolver::GetOkFreestandingOrdered(C_BasePlayer* player, float& ang)
{
	return false;
}

bool Resolver::BackwardsSim(C_BasePlayer* player, float& ang)
{
	return false;
}

bool Resolver::AtTargetSim(C_BasePlayer* player, float& ang)
{
	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
		return false;

	Vector eye_position = Globals::LocalPlayer->GetEyePos();

	Vector target_position = player->GetEyePos();
	ang = Math::CalcAngle(target_position, eye_position).yaw - 180.f;

	return true;
}

void Resolver::UpdateDetectionData(C_BasePlayer* player, int i)
{
	DetectionData[i].AllLbys.push_back(player->m_flLowerBodyYawTarget());
	DetectionData[i].LastAddSimtime.push_back(player->m_flSimulationTime());
	DetectionData[i].LastSimtime = player->m_flSimulationTime();

	if (DetectionData[i].AllLbys.size() >= 2)
		DetectionData[i].WasMoving = false;

	if ((int)DetectionData[i].AllLbys.size() > 4)
		DetectionData[i].AllLbys.erase(DetectionData[i].AllLbys.begin());
}

bool Resolver::IsUsingSpinbot(C_BasePlayer* player, int i, float min_range, float tolerance)
{
	if (DetectionData[i].WasMoving)
		return DetectionData[i].BeforeMoving == DetectionModes::Spinning;

	if (DetectionData[i].AllLbys.size() < 2 || IsStaticAngle(player, i))
		return false;

	//bool IsSpin = false;
	float LastLby = 0.f;
	float AverageChange = 0.f;
	float LastChange = 0.f;

	for (auto lby = DetectionData[i].AllLbys.begin(); lby != DetectionData[i].AllLbys.end(); lby++)
	{
		if (LastLby == 0.f)
		{
			LastLby = *lby;
			continue;
		}

		float CurrentChange = LastLby - *lby;
		AverageChange += CurrentChange;

		if (LastChange == 0.f)
		{
			LastChange = CurrentChange;
			continue;
		}

		if (fabs(LastChange - CurrentChange) > tolerance)
			return false;
	}

	AverageChange /= DetectionData[i].AllLbys.size();

	DetectionData[i].SpinSpeed = AverageChange;

	if (fabs(AverageChange) > min_range)
		return true;

	return false;
}

bool Resolver::IsStaticAngle(C_BasePlayer* player, int i, float tolerance)
{
	if (DetectionData[i].WasMoving)
		return DetectionData[i].BeforeMoving == DetectionModes::Static;

	if (DetectionData[i].AllLbys.size() < 2)
		return true;

	bool IsStatic = true;
	float LastLby = 0.f;

	for (auto lby = DetectionData[i].AllLbys.begin(); lby != DetectionData[i].AllLbys.end(); lby++)
	{
		if (!IsStatic)
			continue;

		if (LastLby == 0.f)
			LastLby = *lby;

		if (fabs(LastLby - *lby) > tolerance)
			IsStatic = false;
	}

	return IsStatic;
}

bool Resolver::IsStaticSwitchOrJitter(C_BasePlayer* player, int i, float tolerance)
{
	//if (DetectionData[i].WasMoving) return DetectionData[i].BeforeMoving == DetectionModes::Random;
	if (DetectionData[i].AllLbys.size() < 2 || IsStaticAngle(player, i))
		return false;

	/*
	bool IsStatic = true;
	float LastLby = 0.f;
	for (auto lby = DetectionData[i].AllLbys.begin(); lby != DetectionData[i].AllLbys.end(); lby++)
	{
		if (!IsStatic) continue;
		if (LastLby == 0.f) LastLby = *lby;

		if (fabs(LastLby - *lby) > tolerance) IsStatic = false;
	}
	*/

	return false;
}

bool Resolver::IsRandomAngle(C_BasePlayer* player, int i, float tolerance)
{
	if (DetectionData[i].WasMoving)
		return DetectionData[i].BeforeMoving == DetectionModes::Random;

	return !IsStaticAngle(player, i) && !IsUsingSpinbot(player, i, 35.f, tolerance);
}
#else
void Resolver::AddCurrentYaw(C_BasePlayer* pl, int i)
{
	LastYaws[i].push_back(pl->m_angEyeAngles().yaw);

	if (LastYaws[i].size() > 8)
		LastYaws[i].erase(LastYaws[i].begin());
}
bool Resolver::IsStaticYaw(int i)
{
	if (LastYaws[i].size() < 3)
		return true;

	//float LastYaw = 0.f;
	//float LastYaw2 = 0.f;
	//bool ReturnVal = true;

	float HighestDifference = 0.f;

	for (size_t p = 0; p < LastYaws[i].size(); p++)
	{
		for (size_t p2 = 0; p2 < LastYaws[i].size(); p2++)
		{
			float c = fabs(fabs(LastYaws[i].at(p)) - fabs(LastYaws[i].at(p2)));

			if (c > HighestDifference)
				HighestDifference = c;
		}
	}

	if (HighestDifference > 15.f)
		return false;
	else
		return true;
}
bool Resolver::GetAverageYaw(int i, float& ang)
{
	if (LastYaws[i].size() < 3)
		return true;

	float add = 0.f;

	for (size_t p = 0; p < LastYaws[i].size(); p++)
		add += LastYaws[i].at(p);

	ang = add / LastYaws[i].size();
	return true;
}
#endif// FakeAnglesEnabled

Resolver g_Resolver;
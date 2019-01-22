#include "backtrack.hpp"
#include "../visuals/visuals.hpp"

inline float point_to_line(Vector Point, Vector LineOrigin, Vector Dir)
{
	auto PointDir = Point - LineOrigin;

	auto TempOffset = PointDir.Dot(Dir) / (Dir.x*Dir.x + Dir.y*Dir.y + Dir.z*Dir.z);
	if (TempOffset < 0.000001f)
		return FLT_MAX;

	auto PerpendicularPoint = LineOrigin + (Dir * TempOffset);

	return (Point - PerpendicularPoint).Length();
}

void CBacktrack::RunLegit(CUserCmd* cmd) //phook backtrack muahhahahahaaha :(
{
	if (Vars.ragebot_backtrack)
		return;

	if (!Vars.legit_backtracking)
		return;

	int bestTargetIndex = -1;
	float bestFov = FLT_MAX;
	player_info_t info;

	if (!Globals::LocalPlayer) return;
	if (Globals::LocalPlayer->m_iHealth() <= 0) return;

	for (int i = 1; i < 65; i++)
	{
		C_BasePlayer* entity = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(i);

		if (!entity) continue;
		if (!Interfaces::Engine->GetPlayerInfo(i, &info)) continue;
		if (entity->IsDormant()) continue;
		if (entity->IsTeammate() && Vars.legit_ignoreteam) continue;
		if (entity->m_iHealth() <= 0) continue;

		float simtime = entity->m_flSimulationTime(); 
		Vector hitboxPos = entity->GetHitboxPos(0);
		Vector origin = entity->m_vecOrigin();
		matrix3x4_t bone_matrix[128];

		for (int i = 0; i < 128; i++)
			bone_matrix[i] = entity->GetBoneMatrix(i);

		headPositions[i][cmd->command_number % 12] = legit_backtrackdata{ simtime, hitboxPos, origin, bone_matrix[128] };

		Vector ViewDir = Math::angle_vector(cmd->viewangles + (Globals::LocalPlayer->m_aimPunchAngle() * 2.f));
		float FOVDistance = point_to_line(hitboxPos, Globals::LocalPlayer->GetEyePos(), ViewDir);

		if (bestFov > FOVDistance)
		{
			bestFov = FOVDistance;
			bestTargetIndex = i;
		}
	}

	float bestTargetSimTime;
	if (bestTargetIndex != -1)
	{
		float tempFloat = FLT_MAX;
		Vector ViewDir = Math::angle_vector(cmd->viewangles + (Globals::LocalPlayer->m_aimPunchAngle() * 2.f));

		for (int t = 0; t < 12; ++t)
		{
			float tempFOVDistance = point_to_line(headPositions[bestTargetIndex][t].hitboxPos, Globals::LocalPlayer->GetEyePos(), ViewDir);
			if (tempFloat > tempFOVDistance && headPositions[bestTargetIndex][t].simtime > Globals::LocalPlayer->m_flSimulationTime() - 1)
			{
				tempFloat = tempFOVDistance;
				bestTargetSimTime = headPositions[bestTargetIndex][t].simtime;
			}
		}

		if (cmd->buttons & IN_ATTACK)
			cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime);
	}
} 

void CBacktrack::paint_traverse() 
{
	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive())
		return;

	if (!Vars.legit_backtracking)
		return;

	for (int i = 1; i < Interfaces::GlobalVars->maxClients; i++) {
		auto m_player = static_cast<C_BasePlayer *>(Interfaces::EntityList->GetClientEntity(i));

		if (!m_player) continue;
		if (m_player->IsDormant()) continue;
		if (m_player->IsTeammate()) continue;
		if (m_player->m_iHealth() <= 0) continue;
		if (Globals::LocalPlayer->m_iHealth() <= 0) return;

		for (int j = 0; j < 12; j++)
		{
			if (headPositions[i][j].simtime <= Globals::LocalPlayer->m_flSimulationTime() - 1)
				continue;
			CVisuals::Get().DrawSkeleton(m_player, Color::White, headPositions[i][j].bone_matrix);
		}
	}
}

void CBacktrack::AddLatency(INetChannel* net_channel, float latency)
{
	for (const auto& sequence : sequence_records)
	{
		if (Interfaces::GlobalVars->realtime - sequence.time >= latency)
		{
			net_channel->m_nInReliableState = sequence.in_reliable_state;
			net_channel->m_nInSequenceNr = sequence.in_sequence_num;
			break;
		}
	}
}

float CBacktrack::GetLerpTime()
{
	int ud_rate = g_CVar->FindVar("cl_updaterate")->GetInt();
	ConVar* min_ud_rate = g_CVar->FindVar("sv_minupdaterate");
	ConVar* max_ud_rate = g_CVar->FindVar("sv_maxupdaterate");

	//Console.WriteLine(min_ud_rate->GetFlags)

	if (min_ud_rate && max_ud_rate)
	{
		ud_rate = max_ud_rate->GetInt();
	}

	float ratio = g_CVar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
	{
		ratio = 1.0f;
	}

	float lerp = g_CVar->FindVar("cl_interp")->GetFloat();
	ConVar* c_min_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
	ConVar* c_max_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");

	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
	{
		ratio = std::clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());
	}

	return std::max<float>(lerp, (ratio / ud_rate));
}

void CBacktrack::UpdateIncomingSequences()
{
	auto nci = reinterpret_cast<INetChannel*>(Interfaces::ClientState->m_NetChannel);
	if (!nci)
		return;

	if (nci->m_nInSequenceNr > last_incoming_sequence || !nci->m_nInSequenceNr)
	{
		last_incoming_sequence = nci->m_nInSequenceNr;

		sequence_records.push_front(Incoming_Sequence_Record(nci->m_nInReliableState, nci->m_nOutReliableState, nci->m_nInSequenceNr, Interfaces::GlobalVars->realtime));
	}

	if (sequence_records.size() > 2048)
		sequence_records.pop_back();
}

legit_backtrackdata headPositions[64][12];
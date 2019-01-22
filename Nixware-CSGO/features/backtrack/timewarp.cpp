#include "timewarp.hpp"
#include "../../options.hpp"
#include <thread>
#include <mutex>
#include <iostream>
//INetChannel* client1 = (INetChannel*)g_ClientState;
using namespace std;
#define PI 3.14159265358979323846f
#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) ( g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define FLOW_OUTGOING    0
#define FLOW_INCOMING    1
std::deque<ÑTimeWarp::SequenceData>twsequences;

template<class T, class U>
T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

float ÑTimeWarp::GetLerpTime()
{
	int ud_rate = g_CVar->FindVar("cl_updaterate")->GetInt();
	ConVar *min_ud_rate = g_CVar->FindVar("sv_minupdaterate");
	ConVar *max_ud_rate = g_CVar->FindVar("sv_maxupdaterate");

	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetInt();

	float ratio = g_CVar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = g_CVar->FindVar("cl_interp")->GetFloat();
	ConVar *c_min_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
	ConVar *c_max_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");

	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

	return max(lerp, (ratio / ud_rate));
} 

void ÑTimeWarp::AddLatencyToNetchan(INetChannel *netchan, float Latency)
{
	for (auto& seq : twsequences)
	{
		if (g_GlobalVars->realtime - seq.Realtime >= Latency)
		{
			netchan->m_nInReliableState = seq.inreliablestate;
			//netchan->m_nOutReliableState = seq.outreliablestate;
			netchan->m_nInSequenceNr = seq.sequencenr;
			break;
		}
	}
}
bool ÑTimeWarp::IsTickValid(int tick)
{
	INetChannelInfo *nci = g_EngineClient->GetNetChannelInfo();

	static auto sv_maxunlag = g_CVar->FindVar("sv_maxunlag");

	if (!nci || !sv_maxunlag)
		return false;

	float correct = clamp(nci->GetLatency(0) + GetLerpTime(), 0.f, sv_maxunlag->GetFloat());

	float deltaTime = correct - (g_GlobalVars->curtime - TICKS_TO_TIME(tick));

	return fabsf(deltaTime) < 0.2f;
}

inline Vector angle_vector(Vector meme)
{
	auto sy = sin(meme.y / 180.f * static_cast<float>(PI));
	auto cy = cos(meme.y / 180.f * static_cast<float>(PI));

	auto sp = sin(meme.x / 180.f * static_cast<float>(PI));
	auto cp = cos(meme.x / 180.f* static_cast<float>(PI));

	return Vector(cp*cy, cp*sy, -sp);
}
int lastincomingsequencenumber = 0;
void ÑTimeWarp::UpdateIncomingSequences() {
	if (!g_ClientState || g_ClientState == 0)
		return;
	DWORD ClientState = (DWORD)*(DWORD*)g_ClientState;
	if (ClientState)
	{
		INetChannel* netchan = (INetChannel*)*(DWORD*)(ClientState + 0x9C);
		if (netchan) {
			if (netchan->m_nInSequenceNr > lastincomingsequencenumber) {
				lastincomingsequencenumber = netchan->m_nInSequenceNr;
				twsequences.push_front(ÑTimeWarp::SequenceData(netchan->m_nInReliableState, netchan->m_nOutReliableState, netchan->m_nInSequenceNr, g_GlobalVars->realtime));
			}
			if (twsequences.size() > 2048)
				twsequences.pop_back();
		}
	}
}
inline float distance_point_to_line(Vector Point, Vector LineOrigin, Vector Dir)
{
	auto PointDir = Point - LineOrigin;

	auto TempOffset = PointDir.Dot(Dir) / (Dir.x*Dir.x + Dir.y*Dir.y + Dir.z*Dir.z);
	if (TempOffset < 0.000001f)
		return FLT_MAX;

	auto PerpendicularPoint = LineOrigin + (Dir * TempOffset);

	return (Point - PerpendicularPoint).Length();
}

void ÑTimeWarp::StoreRecords(CUserCmd* cmd, C_BasePlayer* local)
{
	for (int i = 1; i <= 64; i++)
	{
		if (validRecords.at(i).size() > 0)
			validRecords.at(i).clear();

		auto entity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (!entity || entity->m_iTeamNum() == local->m_iTeamNum() || local == entity || !entity->m_iHealth() || entity->IsDormant())
			continue;

		StoredData record = StoredData(entity);
		viableRecords.at(i).push_front(record);

		if (viableRecords.at(i).size() > 0)
			for (auto k = viableRecords.at(i).begin(); k != viableRecords.at(i).end(); k++)
				if (IsTickValid(TIME_TO_TICKS(k->simtime)))
					validRecords.at(i).push_back(*k);

		if (viableRecords.at(i).size() > 64)
			viableRecords.at(i).pop_back();
	}
}

void ÑTimeWarp::DoBackTrack(CUserCmd* cmd, C_BasePlayer* pLocal)
{
	float bestFov = 180.f, bestFovPlayer = 180.f;
	int bestTarget = -1, bestTargetBT = -1;
	StoredData* bestRecord = nullptr;
	if (!pLocal || !pLocal->IsAlive()) return;

	QAngle tempAngle = cmd->viewangles + pLocal->m_aimPunchAngle() * 2.f;
	Vector tempVec = Vector(tempAngle.pitch, tempAngle.yaw, tempAngle.roll);
	Vector ViewDir = angle_vector(tempVec);

	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* entity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (!entity || entity == pLocal || entity->IsDormant() || !entity->IsAlive()) continue;

		float tempFOVDistance = distance_point_to_line(entity->GetBonePos(8), pLocal->GetEyePos(), ViewDir);
		if (bestFovPlayer > tempFOVDistance)
		{
			bestFovPlayer = tempFOVDistance;
			bestTarget = i;
		}
		for (auto& record : validRecords.at(i))
		{
			float tempFOVDistance2 = distance_point_to_line(record.hitboxPos, pLocal->GetEyePos(), ViewDir);
			if (bestFov > tempFOVDistance2)
			{
				bestFov = tempFOVDistance2;
				bestTargetBT = i;
				bestRecord = &record;
			}
		}
	}
	if (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2)
		if (bestFov < bestFovPlayer && bestRecord && Vars.ragebot_backtrack)
			cmd->tick_count = TIME_TO_TICKS(bestRecord->simtime + GetLerpTime());
}
void ÑTimeWarp::ClearIncomingSequences()
{
	twsequences.clear();
}

ÑTimeWarp* backtracking = new ÑTimeWarp();
std::array<std::deque<StoredData>, 65> validRecords;
std::array<std::deque<StoredData>, 65> viableRecords;
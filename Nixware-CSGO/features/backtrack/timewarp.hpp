#pragma once
#include "../../valve_sdk/csgostructs.hpp"
#include "../ragebot/ragebot.hpp"
#include <array>
#include <deque>

struct StoredData
{
	float simtime = -1;
	Vector hitboxPos = Vector{ 0.f, 0.f, 0.f };
	matrix3x4_t matrix[MAXSTUDIOBONES];
	bool matrixbuilt = false;
	float m_flPoseParameter[24];

	StoredData(C_BasePlayer* ent) {
		simtime = ent->m_flSimulationTime();
		hitboxPos = ent->GetBonePos(8);
		if (ent->SetupBones(matrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, g_GlobalVars->curtime))
			matrixbuilt = true;
	}
};


class ÑTimeWarp : public Singleton<ÑTimeWarp>
{
public:
	struct SequenceData {
		SequenceData::SequenceData(int instate, int outstate, int seqnr, float realtime) {
			inreliablestate = instate;
			outreliablestate = outstate;
			sequencenr = seqnr;
			Realtime = realtime;
		}
		int inreliablestate;
		int outreliablestate;
		int sequencenr;
		float Realtime;
	};
	void StoreRecords(CUserCmd* cmd, C_BasePlayer* local);
	void DoBackTrack(CUserCmd* cmd, C_BasePlayer* pLocal);
	void ClearIncomingSequences();
	float GetLerpTime();
	void AddLatencyToNetchan(INetChannel * netchan, float Latency);
	bool IsTickValid(int tick);
	void UpdateBacktrackRecords(C_BasePlayer * pPlayer);
	void UpdateIncomingSequences();
};
extern ÑTimeWarp* backtracking;
extern std::array<std::deque<StoredData>, 65> validRecords;
extern std::array<std::deque<StoredData>, 65> viableRecords;
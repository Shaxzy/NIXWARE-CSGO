#pragma once
#include <deque>
#include "../../options.hpp"
#include "../../helpers/math.hpp"
#include "../../helpers/utils.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / Interfaces::GlobalVars->interval_per_tick ) )

struct legit_backtrackdata
{
	float simtime;
	Vector hitboxPos;
	Vector origin;
	matrix3x4_t bone_matrix[128];
};

struct Incoming_Sequence_Record
{
	Incoming_Sequence_Record(int in_reliable, int out_reliable, int in_sequence, float realtime)
	{
		in_reliable_state = in_reliable;
		out_reliable_state = out_reliable;
		in_sequence_num = in_sequence;

		time = realtime;
	}

	int in_reliable_state;
	int out_reliable_state;
	int in_sequence_num;

	float time;
};

class CBacktrack : public Singleton<CBacktrack>
{
public:
	void RunLegit(CUserCmd * cmd);
	void paint_traverse();
	void AddLatency(INetChannel * net_channel, float latency);
	float GetLerpTime();
	bool IsTickValid(int tick);
	void UpdateIncomingSequences();
	std::deque<Incoming_Sequence_Record> sequence_records;
	int last_incoming_sequence = 0;
	INetChannel* netchan;
};

extern legit_backtrackdata headPositions[64][12];

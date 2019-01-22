#pragma once
#include <deque>

#include "../../helpers/singleton.hpp"
#include "../../valve_sdk/csgostructs.hpp"

struct AASimulationStruct
{
	bool CanUseFreestanding = false;
	float FreestandAng = 0.f;
	bool CanUseEdge = false;
	float EdgeAngle = 0.f;
	bool GotWallDistance = false;
	float WallDistance = 0.f;
	bool CanUseBackwards = false;
	float BackwardsAng = 0.f;
};

struct CurrentStandAADataStruct
{
	bool UsedEdge = false;
	int LastEdgeShots = 0;
	int FreestandShots = 0;
};

struct ResolverAnimDataStorage
{
	void UpdateAnimationLayers(C_BasePlayer* player)
	{
		m_iLayerCount = player->GetNumAnimOverlays();
		for (int i = 0; i < m_iLayerCount; i++)
		{
			animationLayer[i] = player->GetAnimOverlays()[i];
		}
	};
	int32_t m_iLayerCount;
	AnimationLayer animationLayer[15];
};

struct SavedResolverData
{
	bool WasDormantBetweenMoving = true;
	float LastMovingLby = 0.f;
	float LastCycle = 0.f;
	bool Flip = false;
	float LastSimtime = 0.f;
	float LastResolvedYaw = 0.f;
	bool LastResolved = true;
	bool WasLastMoving = false;
	float LastStandingLby = 0.f;
	float MoveStandDelta = 0.f;
	bool CanUseMoveStandDelta = false;
	bool WasFakeWalking = false;
	//CanUseLbyPrediction
	float LastLby = 0.f;
	float LegitTestLastSimtime = 0.f;
	float lastLbyUpdateTime = 0.f;
	bool UsingAA = false;
	float LastWasUsingAA = 0.f;
	Vector LastPos = Vector(0, 0, 0);
	float FakeYaw = 0.f;
	float LastVel = 0.f;
};

enum class DetectionModes : int
{
	None,
	Spinning,
	Static,
	Random
};
struct ResolverDetectionData
{
	std::deque<float> AllLbys;
	std::deque<float> LastAddSimtime;
	float LastSimtime = 0.f;;
	float SpinSpeed = 0.f;
	DetectionModes mode = DetectionModes::None;
	DetectionModes BeforeMoving = DetectionModes::None;
	bool WasMoving = false;
};

enum class ResolverModes : int
{
	NONE,
	FREESTANDING,
	BACKWARDS,
	EDGE,
	MOVE_STAND_DELTA,
	FORCE_LAST_MOVING_LBY,
	FORCE_FREESTANDING,
	FORCE_BACKWARDS,
	BRUTFORCE_ALL_DISABLED, //todo
	BRUTFORCE,
	FORCE_MOVE_STAND_DELTA,
	FORCE_LBY,
	MOVING,
	LBY_BREAK,
	SPINBOT,
	AIR_FREESTANDING,
	AIR_BRUTFORCE,
	FAKEWALK_FREESTANDING,
	FAKEWALK_BRUTFORCE,
	PREDICT_FREESTANDING,
	PREDICT_BRUTFORCE,
	MAX
};

enum class ResolverDetections : int
{
	NONE,
	FAKEWALKING,
	AIR,
	MOVING,
	STANDING
};

enum class BacktrackPriorities : int
{
	none,
	good_prediction,
	normal_prediction,
	lby_prediction,
	lby_update,
	moving
};

#ifdef FakeAnglesEnabled
struct GlobalResolverDataStruct
{
	bool Resolved = false;
	bool ForceBaim = false;
	int Shots = 0;
	int ResolverState = 0; // bad, resolved, lby update, moving
	ResolverModes mode = ResolverModes::NONE;
	ResolverDetections detection = ResolverDetections::NONE;
	//Shots counter
	//int ShotsAtMoveStandDelta = 0;
	int FakeWalkShots = 0;
	bool InFakelag = false;
	bool Fake = true;
	bool Moving = false;
	bool InAir = false;
	int ShotsAtMode[(int)ResolverModes::MAX];
	int OverallShotsAtMode[(int)ResolverModes::MAX];
	int OverallShots = 0;
	float ResolvedYaw = 0.f;
	BacktrackPriorities BacktrackPriority = BacktrackPriorities::none;
	bool BreakingLC = false; //g_Saver.LCbroken = (Globals::LocalPlayer->m_vecOrigin() - g_Saver.FakelagData.pos).LengthSqr() > 4096.f;
	bool CanuseLbyPrediction = false;
	float NextPredictedLbyBreak = 0.f;
};
#else
struct GlobalResolverDataStruct
{
	bool Resolved = false;
	bool ForceBaim = false;
	int Shots = 0;
	int ResolverState = 0; // bad, resolved, lby update, moving
	ResolverModes mode = ResolverModes::NONE;
	ResolverDetections detection = ResolverDetections::NONE;
	//Shots counter
	//int ShotsAtMoveStandDelta = 0;
	int FakeWalkShots = 0;
	bool InFakelag = false;
	bool Fake = true;
	bool Moving = false;
	bool InAir = false;
	int ShotsAtMode[(int)ResolverModes::MAX];
	int OverallShotsAtMode[(int)ResolverModes::MAX];
	int OverallShots = 0;
	//float ResolvedYaw = 0.f;
	//BacktrackPriorities BacktrackPriority = BacktrackPriorities::none;
	bool BreakingLC = false; //g_Saver.LCbroken = (Globals::LocalPlayer->m_vecOrigin() - g_Saver.FakelagData.pos).LengthSqr() > 4096.f;
	bool CanuseLbyPrediction = false;
	float NextPredictedLbyBreak = 0.f;
};
#endif

extern bool InLc;
extern bool InLbyUpdate;
extern bool Moving;

class Resolver : public Singleton<Resolver>
{
public:
	void OnCreateMove(QAngle OrgViewang);
	void OnFramestageNotify();
	void OnFireEvent(IGameEvent* event);
	std::array<GlobalResolverDataStruct, 128> GResolverData;
	std::array<CurrentStandAADataStruct, 128> StandAAData;
private:
	std::array<AASimulationStruct, 128> SimulatedAAs;
	std::array<SavedResolverData, 128> SavedResolverData;

#ifdef FakeAnglesEnabled
	/* gives back if the angle is valid */
	bool IsValidAngle(C_BasePlayer* player, float angle);

	/* get wall distance */
	bool GetWallDistance(C_BasePlayer* player, float& val);

	/* is fakewalking */
	bool InFakeWalk(C_BasePlayer* player, ResolverAnimDataStorage store);
	bool InFakeWalkOld(C_BasePlayer* player, ResolverAnimDataStorage store);

	/* is in big flick */
	bool Is979(C_BasePlayer* player, ResolverAnimDataStorage store, AnimationLayer& layer);

	/* freestand simulation */
	bool FreestandingSim(C_BasePlayer* player, float& ang);

	/* good freestand angles other than main simulation */
	bool GetOkFreestandingOrdered(C_BasePlayer* player, float& ang);

	/* backwards ang */
	bool BackwardsSim(C_BasePlayer* player, float& ang);

	/* at target ang || backwards emulation */
	bool AtTargetSim(C_BasePlayer* player, float& ang);

	/*
		Detections
	*/
	std::array<ResolverDetectionData, 128> DetectionData;
	void UpdateDetectionData(C_BasePlayer* player, int i);
	/* Spinbot detections */
	bool IsUsingSpinbot(C_BasePlayer* player, int i, float min_range = 35.f, float tolerance = 25.f);
	/* Static detection */
	bool IsStaticAngle(C_BasePlayer* player, int i, float tolerance = 10.f);
	/* Static jitter or switch detection */
	bool IsStaticSwitchOrJitter(C_BasePlayer* player, int i, float tolerance = 90.f);
	/* Random detection */
	bool IsRandomAngle(C_BasePlayer* player, int i, float tolerance = 25.f);
#else
	std::deque<float> LastYaws[128];
	void AddCurrentYaw(C_BasePlayer* pl, int i);
	bool IsStaticYaw(int i);
	bool GetAverageYaw(int i, float& ang);
#endif
};

extern Resolver g_Resolver;
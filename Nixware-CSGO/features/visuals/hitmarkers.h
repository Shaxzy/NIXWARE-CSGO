#pragma once

#include "../../helpers/singleton.hpp"

#include "../../helpers/draw_manager.hpp"
#include "../../helpers/math.hpp"
#include "../../valve_sdk/csgostructs.hpp"

struct pImpactInfo 
{
	float x, y, z;
	long long time;
};

struct pHitmarkerInfo 
{
	pImpactInfo pImpact;
	int alpha;
};

class Hitmarker
{
	public:
		void Paint();
		void OnPlayerHurt(IGameEvent* pEvent);
		void OnBulletImpact(IGameEvent* pEvent);

	private:
		C_BasePlayer* GetPlayer(int userid);
		std::vector<pImpactInfo> pImpacts;
		std::vector<pHitmarkerInfo> pHitmarkers;
};

extern Hitmarker* pHitmarker;
#pragma once

#include "hitmarkers.h"

#include "../../options.hpp"
#include "../../helpers/math.hpp"
#include "../../helpers/utils.hpp"

#include <ctime>
#include <chrono>
#include <cstdint>

Hitmarker* pHitmarker = new Hitmarker();

int64_t GetEpochMS()
{
	int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return now;
}

C_BasePlayer* Hitmarker::GetPlayer(int userid)
{
	int index = Interfaces::Engine->GetPlayerForUserID(userid);
	return (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(index);
}

void Hitmarker::Paint() 
{
	//if (!Settings::settings.hitmarker)
	//	return;

	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame() || !Globals::LocalPlayer)
	{

		if (!pImpacts.empty())
			pImpacts.clear();

		if (!pHitmarkers.empty())
			pHitmarkers.clear();

		return;
	}

	long long pTime = GetEpochMS();

	std::vector<pHitmarkerInfo>::iterator pIter;

	for (pIter = pHitmarkers.begin(); pIter != pHitmarkers.end();) 
	{
		bool pExpired = pTime > pIter->pImpact.time + 3000; // 2000

		static int pAlphaInterval = 255 / 50;

		if (pExpired)
			pIter->alpha -= pAlphaInterval;

		if (pExpired && pIter->alpha <= 0) {

			pIter = pHitmarkers.erase(pIter);
			continue;
		}

		int pLineSize = 6;
		Color pColor(1.f, 1.f, 1.f, ((float)pIter->alpha) / 255.f);

		int w, h;
		Interfaces::Engine->GetScreenSize(w, h); w /= 2; h /= 2;

		CDraw::Get().Line(w - 2, h - 2, w - (pLineSize), h - (pLineSize), pColor);
		CDraw::Get().Line(w - 2, h + 2, w - (pLineSize), h + (pLineSize), pColor);
		CDraw::Get().Line(w + 2, h + 2, w + (pLineSize), h + (pLineSize), pColor);
		CDraw::Get().Line(w + 2, h - 2, w + (pLineSize), h - (pLineSize), pColor);

		Vector pos3D = Vector(pIter->pImpact.x, pIter->pImpact.y, pIter->pImpact.z), pos2D;

		if (!Math::WorldToScreen(pos3D, pos2D)) 
		{
			++pIter;
			continue;
		}

		CDraw::Get().Line(pos2D.x - 2, pos2D.y - 2, pos2D.x - (pLineSize), pos2D.y - (pLineSize), pColor);
		CDraw::Get().Line(pos2D.x - 2, pos2D.y + 2, pos2D.x - (pLineSize), pos2D.y + (pLineSize), pColor);
		CDraw::Get().Line(pos2D.x + 2, pos2D.y + 2, pos2D.x + (pLineSize), pos2D.y + (pLineSize), pColor);
		CDraw::Get().Line(pos2D.x + 2, pos2D.y - 2, pos2D.x + (pLineSize), pos2D.y - (pLineSize), pColor);

		++pIter;
	}
}

void Hitmarker::OnPlayerHurt(IGameEvent* pEvent) 
{
	C_BasePlayer* pAttacker = GetPlayer(pEvent->GetInt("attacker"));
	C_BasePlayer* pVictim = GetPlayer(pEvent->GetInt("userid"));

	if (!pAttacker || !pVictim || pAttacker != Globals::LocalPlayer)
		return;

	Interfaces::Surface->PlaySound_("buttons//arena_switch_press_02.wav");

	Vector pEnemyPos = pVictim->GetEyePos();
	pImpactInfo pBestImpact;

	float pBestImpactDistance = -1;
	long long pTime = GetEpochMS();

	std::vector<pImpactInfo>::iterator pIter;

	for (pIter = pImpacts.begin(); pIter != pImpacts.end();) 
	{
		if (pTime > pIter->time + 25) 
		{
			pIter = pImpacts.erase(pIter);
			continue;
		}

		Vector Position = Vector(pIter->x, pIter->y, pIter->z);

		float pDistance = Position.DistTo(pEnemyPos);

		if (pDistance < pBestImpactDistance || pBestImpactDistance == -1) 
		{
			pBestImpactDistance = pDistance;
			pBestImpact = *pIter;
		}

		pIter++;
	}

	if (pBestImpactDistance == -1)
		return;

	pHitmarkerInfo pInfo;
	pInfo.pImpact = pBestImpact;
	pInfo.alpha = 255;
	pHitmarkers.push_back(pInfo);
}

void Hitmarker::OnBulletImpact(IGameEvent* pEvent) 
{
	C_BasePlayer* pShooter = GetPlayer(pEvent->GetInt("userid"));

	if (!pShooter || pShooter != Globals::LocalPlayer)
		return;

	pImpactInfo pInfo;

	pInfo.x = pEvent->GetFloat("x");
	pInfo.y = pEvent->GetFloat("y");
	pInfo.z = pEvent->GetFloat("z");
	pInfo.time = GetEpochMS();

	pImpacts.push_back(pInfo);
}
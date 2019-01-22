#pragma once

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <initializer_list>
#include "../valve_sdk/sdk.hpp"
#include "../datamap.hpp"
#include "../security/config.h"
#include "../security/VMProtectSDK.h"

typedef void(*LPSEARCHFUNC)(LPCTSTR lpszFileName);

namespace Utils 
{
	extern int pWeaponItemIndexData[34];
	extern std::string pWeaponData[34];

	void CorrectMovement(QAngle vOldAngles, CUserCmd * pCmd, float fOldForward, float fOldSidemove);

	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);

	bool HitChance(float hitchance);

	void ForceFullUpdate();

	unsigned int FindInDataMap(datamap_t * pMap, const char * name);

	bool IsKeyPressed(int iKey);

	int GetWeaponSettingsSelectID();
	bool IsInGame();
	bool IsInDangerZone();
	int GetFps();
	bool LineGoesThroughSmoke(Vector vStartPos, Vector vEndPos);
	bool IsVisible(Vector & vecAbsStart, Vector & vecAbsEnd, IClientEntity * pLocalEntity, IClientEntity * pBaseEntity, bool bSmokeCheck);
	void AttachConsole();
    void DetachConsole();
    bool ConsolePrint(const char* fmt, ...);  
    char ConsoleReadKey();
    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::string>& modules);

    std::uint8_t* PatternScan(PVOID module, const char* signature);
	DWORD FindPattern(std::string moduleName, BYTE * Mask, char * szMask);
	DWORD FindSignature(const char * szModuleName, const char * PatternName, char * szPattern);
	void SetClantag(const char* tag);
    void SetName(const char* name);
	std::vector<int> GetObservervators(int playerId);
    void RankRevealAll();
	void TraceLine(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, const IHandleEntity* ignore, trace_t* ptr);
	void ClipTraceToPlayers(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, trace_t* tr);
}

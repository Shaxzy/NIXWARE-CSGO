#include "Utils.hpp"

#define NOMINMAX
#include <Windows.h>
#include <stdio.h>
#include <chrono>

#include <string>
#include <vector>
#include <Psapi.h>

#include "../valve_sdk/csgostructs.hpp"
#include "Math.hpp"

#define IsInRange(x, a, b) (x >= a && x <= b)
#define GetBits(x) (IsInRange(x, '0', '9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xA))
#define GetByte(x) (GetBits(x[0]) << 4 | GetBits(x[1]))

HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;

namespace Utils 
{
	int pWeaponItemIndexData[34] =
	{
		WEAPON_DEAGLE,WEAPON_DUALBERETTA,WEAPON_FIVESEVEN,
		WEAPON_GLOCK,WEAPON_P2000,WEAPON_P250,
		WEAPON_USPS,WEAPON_CZ75,WEAPON_REVOLVER,
		WEAPON_TEC9,

		WEAPON_AK47,WEAPON_AUG,WEAPON_FAMAS,WEAPON_GALIL,
		WEAPON_M249,WEAPON_M4A4,WEAPON_M4A1S,WEAPON_MAC10,
		WEAPON_P90,WEAPON_MP5SD,WEAPON_UMP45,WEAPON_XM1014,WEAPON_BIZON,
		WEAPON_MAG7,WEAPON_NEGEV,WEAPON_SAWEDOFF,
		WEAPON_MP7,WEAPON_MP9,WEAPON_NOVA,WEAPON_SG553,
		WEAPON_SCAR20,WEAPON_G3SG1,

		WEAPON_AWP,WEAPON_SSG08
	};

	std::string pWeaponData[34] =
	{
		"Desert Eagle","Dual Berettas","Five-SeveN",
		"Glock-18","P2000","P250",
		"USP-S","CZ75 Auto","R8 Revolver",
		"Tec-9",

		"AK-47","AUG","FAMAS","Galil AR",
		"M249","M4A4","M4A1-S","MAC-10"	,
		"P90","MP5-SD","UMP-45","XM1014","PP-Bizon",
		"MAG-7","Negev","Sawed-Off",
		"MP7","MP9","Nova","SG 553",
		"SCAR-20","G3SG1",

		"AWP","SSG 08"
	};

	void CorrectMovement(QAngle vOldAngles, CUserCmd* pCmd, float fOldForward, float fOldSidemove)
	{
		float deltaView = pCmd->viewangles.yaw - vOldAngles.yaw;
		float f1;
		float f2;

		if (vOldAngles.yaw < 0.f)
		{
			f1 = 360.0f + vOldAngles.yaw;
		}
		else
		{
			f1 = vOldAngles.yaw;
		}

		if (pCmd->viewangles.yaw < 0.0f)
		{
			f2 = 360.0f + pCmd->viewangles.yaw;
		}
		else
		{
			f2 = pCmd->viewangles.yaw;
		}

		if (f2 < f1)
		{
			deltaView = abs(f2 - f1);
		}
		else
		{
			deltaView = 360.0f - abs(f1 - f2);
		}
		deltaView = 360.0f - deltaView;

		pCmd->forwardmove = cos(DEG2RAD(deltaView)) * fOldForward + cos(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
		pCmd->sidemove = sin(DEG2RAD(deltaView)) * fOldForward + sin(DEG2RAD(deltaView + 90.f)) * fOldSidemove;

		pCmd->buttons &= ~IN_MOVERIGHT;
		pCmd->buttons &= ~IN_MOVELEFT;
		pCmd->buttons &= ~IN_FORWARD;
		pCmd->buttons &= ~IN_BACK;
	}

	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle)
	{
		QAngle delta = aimAngle - viewAngle;
		Math::NormalizeAngles(delta);
		Math::ClampAngles(delta);
		return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
	}

	bool HitChance(float hitchance)
	{
		if (!Globals::LocalPlayer->m_hActiveWeapon())
			return false;

		if (hitchance > 0 && Globals::LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() != WEAPON_REVOLVER)
		{
			float Inaccuracy = Globals::LocalPlayer->m_hActiveWeapon()->GetInaccuracy();

			if (Inaccuracy == 0)
			{
				Inaccuracy = 0.0000001;
			}

			Inaccuracy = 1 / Inaccuracy;
			return (((hitchance * 1.5f) <= Inaccuracy) ? true : false);
		}

		return true;
	}


	template<class T>
	static T* FindHudElement(const char* name)
	{
		static auto pThis = *reinterpret_cast<DWORD**>(UTILS::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);
		static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(UTILS::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
		return (T*)find_hud_element(pThis, name);
	}

	struct hud_weapons_t {
		std::int32_t* get_weapon_count() {
			return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
		}
	};

	void ForceFullUpdate()
	{
		/*static auto clear_hud_weapon_icon_fn =
			reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(
				UTILS::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C"));
		auto element = FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");
		auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xA0);
		if (hud_weapons == nullptr)
			return;
		if (!*hud_weapons->get_weapon_count())
			return;
		for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
			i = clear_hud_weapon_icon_fn(hud_weapons, i);*/

		Interfaces::ClientState->ForceFullUpdate();
	}

	unsigned int FindInDataMap(datamap_t *pMap, const char *name) {
		while (pMap) {
			for (int i = 0; i<pMap->dataNumFields; i++) {
				if (pMap->dataDesc[i].fieldName == NULL)
					continue;

				if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
					return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

				if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED) {
					if (pMap->dataDesc[i].td) {
						unsigned int offset;

						if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
							return offset;
					}
				}
			}
			pMap = pMap->baseMap;
		}

		return 0;
	}
	bool bKeyPress[256];

	bool IsKeyPressed(int iKey)
	{
		if (GetAsyncKeyState(iKey))
		{
			if (!bKeyPress[iKey])
			{
				bKeyPress[iKey] = true;
				return true;
			}
		}
		else
			bKeyPress[iKey] = false;
		return false;
	}

	int GetWeaponSettingsSelectID()
	{
		for (size_t i = 0; i < (sizeof(pWeaponData) / sizeof(*pWeaponData)); i++)
		{
			if (pWeaponItemIndexData[i] == Globals::LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex())
				return i;
		}

		return -1;
	}

	bool IsInGame()
	{
		return Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame();
	}

	bool IsInDangerZone()
	{
		return Interfaces::GameType->GetCurrentGameType() == 6;
	}

	int GetFps()
	{
		static int count = 0;
		static auto last = std::chrono::high_resolution_clock::now();
		auto now = std::chrono::high_resolution_clock::now();
		static int fps = 0;

		count++;

		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() > 1000) {
			fps = static_cast<int>(1.f / Interfaces::GlobalVars->frametime);;
			count = 0;
			last = now;
		}

		return fps;
	}

	bool LineGoesThroughSmoke(Vector vStartPos, Vector vEndPos)
	{
		static auto LineGoesThroughSmokeFn = (bool(*)(Vector vStartPos, Vector vEndPos))Utils::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), XorStr("55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0"));
		return LineGoesThroughSmokeFn(vStartPos, vEndPos);
	}

	bool IsVisible(Vector& vecAbsStart, Vector& vecAbsEnd, IClientEntity* pLocalEntity, IClientEntity* pBaseEntity, bool bSmokeCheck = true)
	{
		trace_t tr;
		Ray_t ray;
		CTraceFilter filter;
		filter.pSkip = pLocalEntity;
		ray.Init(vecAbsStart, vecAbsEnd);

		Interfaces::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

		if (tr.allsolid || tr.startsolid)
			return false;

		bool visible = (pBaseEntity != NULL && tr.hit_entity == pBaseEntity || tr.fraction >= 0.96f);

		if (visible && bSmokeCheck)
			visible = !LineGoesThroughSmoke(vecAbsStart, vecAbsEnd);

		return visible;
	}

    void AttachConsole()
    {
        _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
        _old_err = GetStdHandle(STD_ERROR_HANDLE);
        _old_in  = GetStdHandle(STD_INPUT_HANDLE);

        ::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

        _out     = GetStdHandle(STD_OUTPUT_HANDLE);
        _err     = GetStdHandle(STD_ERROR_HANDLE);
        _in      = GetStdHandle(STD_INPUT_HANDLE);

        SetConsoleMode(_out,
            ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

        SetConsoleMode(_in,
            ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
            ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
    }

    void DetachConsole()
    {
        if(_out && _err && _in) {
            FreeConsole();

            if(_old_out)
                SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
            if(_old_err)
                SetStdHandle(STD_ERROR_HANDLE, _old_err);
            if(_old_in)
                SetStdHandle(STD_INPUT_HANDLE, _old_in);
        }
    }

    bool ConsolePrint(const char* fmt, ...)
    {
        if(!_out) 
            return false;

        char buf[1024];
        va_list va;

        va_start(va, fmt);
        _vsnprintf_s(buf, 1024, fmt, va);
        va_end(va);

        return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
    }

    char ConsoleReadKey()
    {
        if(!_in)
            return false;

        auto key = char{ 0 };
        auto keysread = DWORD{ 0 };

        ReadConsoleA(_in, &key, 1, &keysread, nullptr);

        return key;
    }

    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::string>& modules)
    {
        bool signaled[32] = { 0 };
        bool success = false;

        std::uint32_t totalSlept = 0;

        if(timeout == 0) {
            for(auto& mod : modules) {
                if(GetModuleHandleA(std::data(mod)) == NULL)
                    return WAIT_TIMEOUT;
            }
            return WAIT_OBJECT_0;
        }

        if(timeout < 0)
            timeout = INT32_MAX;

        while(true) {
            for(auto i = 0u; i < modules.size(); ++i) {
                auto& module = *(modules.begin() + i);
                if(!signaled[i] && GetModuleHandleA(std::data(module)) != NULL) {
                    signaled[i] = true;

                    //
                    // Checks if all modules are signaled
                    //
                    bool done = true;
                    for(auto j = 0u; j < modules.size(); ++j) {
                        if(!signaled[j]) {
                            done = false;
                            break;
                        }
                    }
                    if(done) {
                        success = true;
                        goto exit;
                    }
                }
            }
            if(totalSlept > std::uint32_t(timeout)) {
                break;
            }
            Sleep(10);
            totalSlept += 10;
        }

    exit:
        return success ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    }

    std::uint8_t* PatternScan(void* module, const char* signature)
    {
        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for(auto current = start; current < end; ++current) {
                if(*current == '?') {
                    ++current;
                    if(*current == '?')
                        ++current;
                    bytes.push_back(-1);
                } else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for(auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for(auto j = 0ul; j < s; ++j) {
                if(scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if(found) {
                return &scanBytes[i];
            }
        }
        return nullptr;
    }

	DWORD WaitOnModuleHandle(std::string moduleName)
	{
		DWORD ModuleHandle = NULL;
		while (!ModuleHandle)
		{
			ModuleHandle = (DWORD)GetModuleHandleA(moduleName.c_str());
			if (!ModuleHandle)
				Sleep(50);
		}
		return ModuleHandle;
	}

	bool bCompare(const BYTE* Data, const BYTE* Mask, const char* szMask)
	{
		for (; *szMask; ++szMask, ++Mask, ++Data)
		{
			if (*szMask == 'x' && *Mask != *Data)
			{
				return false;
			}
		}
		return (*szMask) == 0;
	}

	DWORD FindPattern(std::string moduleName, BYTE* Mask, char* szMask)
	{
		DWORD Address = WaitOnModuleHandle(moduleName.c_str());
		MODULEINFO ModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)Address, &ModInfo, sizeof(MODULEINFO));
		DWORD Length = ModInfo.SizeOfImage;
		for (DWORD c = 0; c < Length; c += 1)
		{
			if (bCompare((BYTE*)(Address + c), Mask, szMask))
			{
				return DWORD(Address + c);
			}
		}
		return 0;
	}

	DWORD FindSig(DWORD dwAddress, DWORD dwLength, const char* szPattern)
	{
		if (!dwAddress || !dwLength || !szPattern)
			return 0;

		const char* pat = szPattern;
		DWORD firstMatch = NULL;

		for (DWORD pCur = dwAddress; pCur < dwLength; pCur++)
		{
			if (!*pat)
				return firstMatch;

			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GetByte(pat))
			{
				if (!firstMatch)
					firstMatch = pCur;

				if (!pat[2])
					return firstMatch;

				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
					pat += 3;

				else pat += 2;
			}
			else
			{
				pat = szPattern;
				firstMatch = 0;
			}
		}

		return 0;
	}

	DWORD FindSignature(const char* szModuleName, const char* PatternName, char* szPattern)
	{
		HMODULE hModule = GetModuleHandleA(szModuleName);
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
		PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hModule) + pDOSHeader->e_lfanew);

		DWORD ret = FindSig(((DWORD)hModule) + pNTHeaders->OptionalHeader.BaseOfCode, ((DWORD)hModule) + pNTHeaders->OptionalHeader.SizeOfCode, szPattern);;

		return ret;
	}

    void SetClantag(const char* tag)
    {
        static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*))PatternScan(GetModuleHandleW(L"engine.dll"), "53 56 57 8B DA 8B F9 FF 15");

        fnClantagChanged(tag, tag);
    }

    void SetName(const char* name)
    {
        static auto nameConvar = Interfaces::Convar->FindVar("name");
        nameConvar->m_fnChangeCallbacks.m_Size = 0;

        static auto do_once = (nameConvar->SetValue("\nญญญ"), true);

        nameConvar->SetValue(name);
    }

	std::vector<int> GetObservervators(int playerId)
	{
		std::vector<int> SpectatorList;

		C_BasePlayer* pPlayer = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(playerId);

		if (!pPlayer)
			return SpectatorList;

		if (!pPlayer->IsAlive())
		{
			C_BasePlayer* pObserverTarget = (C_BasePlayer*)Interfaces::EntityList->GetClientEntityFromHandle(pPlayer->m_hObserverTarget());

			if (!pObserverTarget)
				return SpectatorList;

			pPlayer = pObserverTarget;
		}

		for (int PlayerIndex = 0; PlayerIndex < 65; PlayerIndex++)
		{
			C_BasePlayer* pCheckPlayer = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(PlayerIndex);

			if (!pCheckPlayer)
				continue;

			if (pCheckPlayer->IsDormant() || pCheckPlayer->IsAlive())
				continue;

			C_BasePlayer* pObserverTarget = (C_BasePlayer*)Interfaces::EntityList->GetClientEntityFromHandle(pCheckPlayer->m_hObserverTarget());

			if (!pObserverTarget)
				continue;

			if (pPlayer != pObserverTarget)
				continue;

			SpectatorList.push_back(PlayerIndex);
		}

		return SpectatorList;
	}

	void RankRevealAll()
	{
		using ServerRankRevealAll = char(__cdecl*)(int*);

		static uint8_t* fnServerRankRevealAll = PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 8B 0D ? ? ? ? 85 C9 75 ? A1 ? ? ? ? 68 ? ? ? ? 8B 08 8B 01 FF 50 ? 85 C0 74 ? 8B C8 E8 ? ? ? ? 8B C8 EB ? 33 C9 89 0D ? ? ? ? 8B 45 ? FF 70 ? E8 ? ? ? ? B0 ? 5D");

		if (fnServerRankRevealAll) {
			int v[3] = { 0,0,0 };

			reinterpret_cast<ServerRankRevealAll>(fnServerRankRevealAll)(v);
		}
	}

	void TraceLine
	(
		Vector& vecAbsStart, 
		Vector& vecAbsEnd, 
		unsigned int mask, 
		const IHandleEntity* ignore, 
		trace_t* ptr
	) 
	{
		Ray_t ray;
		ray.Init(vecAbsStart, vecAbsEnd);
		CTraceFilter filter;
		filter.pSkip = (PVOID)ignore;

		Interfaces::EngineTrace->TraceRay(ray, mask, &filter, ptr);
	}

	float DistanceToRay(const Vector& pos, const Vector& rayStart, const Vector& rayEnd, float* along = nullptr, Vector* pointOnRay = nullptr)
	{
		Vector to = pos - rayStart;
		Vector dir = rayEnd - rayStart;

		dir.NormalizeInPlace();

		float length = dir.Length();

		float rangeAlong = dir.Dot(to);

		if (along)
		{
			*along = rangeAlong;
		}

		float range;

		if (rangeAlong < 0.0f)
		{
			// off start point
			range = -(pos - rayStart).Length();

			if (pointOnRay)
			{
				*pointOnRay = rayStart;
			}
		}
		else if (rangeAlong > length)
		{
			// off end point
			range = -(pos - rayEnd).Length();

			if (pointOnRay)
			{
				*pointOnRay = rayEnd;
			}
		}
		else // within ray bounds
		{
			Vector onRay = rayStart + (dir.operator*(rangeAlong));
			range = (pos - onRay).Length();

			if (pointOnRay)
			{
				*pointOnRay = onRay;
			}
		}

		return range;
	}

	//void ClipTraceToPlayers(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, trace_t* tr) 
	//{
	//	trace_t playerTrace;
	//	Ray_t ray;
	//	float smallestFraction = tr->fraction;
	//	const float maxRange = 60.0f;

	//	ray.Init(vecAbsStart, vecAbsEnd);

	//	for (int k = 1; k <= g_GlobalVars->maxClients; ++k)
	//	{
	//		C_BasePlayer* player = (C_BasePlayer*)g_EntityList->GetClientEntity(k);

	//		if (!player || !player->IsAlive()) continue;

	//		if (player->IsDormant()) continue;

	//		if (filter && filter->ShouldHitEntity(player, mask) == false)
	//			continue;

	//		float range = DistanceToRay(player->GetBaseEntity()->world, vecAbsStart, vecAbsEnd);
	//		if (range < 0.0f || range > maxRange)
	//			continue;

	//		Interfaces.pTrace->ClipRayToEntity(ray, mask | CONTENTS_HITBOX, player, &playerTrace);
	//		if (playerTrace.fraction < smallestFraction)
	//		{
	//			// we shortened the ray - save off the trace
	//			*tr = playerTrace;
	//			smallestFraction = playerTrace.fraction;
	//		}
	//	}
	//}
}
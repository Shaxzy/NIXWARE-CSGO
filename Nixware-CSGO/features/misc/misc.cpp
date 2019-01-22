#include "misc.hpp"
#include "../../options.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include "../../imgui/imgui.h"
#include "../../hooks.hpp"
#include "../../helpers/input.hpp"
#include "../../helpers/math.hpp"

std::string clantag_ = (XorStr("nixware.cc "));

void CMisc::SetNorecoilAngles(ClientFrameStage_t stage, QAngle* aim_punch, QAngle* view_punch, QAngle old_aim_punch, QAngle old_view_punch)
{
	if (stage != FRAME_RENDER_START)
		return;

	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive())
		return;

	if (Interfaces::Input->m_fCameraInThirdPerson)
		return;

	if (true)
	{
		aim_punch = &old_aim_punch;
		view_punch = &old_view_punch;
	}
}

void CMisc::NoVisualRecoil(ClientFrameStage_t stage, QAngle* aim_punch, QAngle* view_punch, QAngle old_aim_punch, QAngle old_view_punch)
{
	if (stage != FRAME_RENDER_START)
		return;

	if (!Globals::LocalPlayer)
		return;

	if (!Globals::LocalPlayer->IsAlive())
		return;

	if (Interfaces::Input->m_fCameraInThirdPerson)
		return;

	if (view_punch && aim_punch/* && VisualElements.Visual_Misc_NoRecoil->Checked*/)
	{
		old_view_punch = *view_punch;
		old_aim_punch = *aim_punch;

		view_punch->Init();
		aim_punch->Init();
	}
}

void CMisc::SetThirdpersonAngles(ClientFrameStage_t stage)
{
	if (stage != FRAME_RENDER_START)
		return;

	if (Utils::IsInGame() && Globals::LocalPlayer)
	{
		if (Globals::LocalPlayer->IsAlive() && Interfaces::Input->m_fCameraInThirdPerson)
			Globals::LocalPlayer->SetVAngles(Globals::LastAngle);
	}
}

void CMisc::FakeLag(CUserCmd* cmd, bool& bSendPacket)
{
	if (!Globals::LocalPlayer || !Globals::LocalPlayer->IsAlive()) return;
	
	int choked_commands = Interfaces::ClientState->chokedcommands + 1;
	static bool WasLastInFakelag = false;

	bool Moving = Globals::LocalPlayer->m_vecVelocity().Length2D() > 0.1f || (cmd->sidemove != 0.f || cmd->forwardmove != 0.f);
	bool InAir = !(Globals::LocalPlayer->m_fFlags() & FL_ONGROUND);
	bool Standing = !Moving && !InAir;
	int ticks = Vars.ragebot_fakelag_amt;
	int mode = 0;

	if (ticks == 0)	
		return;
	
	switch (mode)
	{
	case 0:	
		if (choked_commands <= ticks)
		{
			WasLastInFakelag = true;
			bSendPacket = false;
		}
		else
		{
			WasLastInFakelag = false;
		}
		break;	
	case 1:	
		int PacketsToChoke = 0;
		if (Globals::LocalPlayer->m_vecVelocity().Length() > 0.f)
		{
			PacketsToChoke = (int)(64.f / Interfaces::GlobalVars->interval_per_tick / Globals::LocalPlayer->m_vecVelocity().Length()) + 1;
			if (PacketsToChoke >= 16)
			{
				PacketsToChoke = 15;
			}

			if (PacketsToChoke >= ticks)
			{
				PacketsToChoke = ticks;
			}
		}

		if (choked_commands <= PacketsToChoke)
		{
			WasLastInFakelag = true;
			bSendPacket = false;
		}
		else
		{
			WasLastInFakelag = false;
		}
		break;	
	}
}

void CMisc::SlowWalk(CUserCmd *cmd)
{
	if (Vars.ragebot_slowwalk_amt <= 0 || !GetAsyncKeyState(Vars.ragebot_slowwalk_key))
		return;

	auto weapon_handle = Globals::LocalPlayer->m_hActiveWeapon();

	if (!weapon_handle)
		return;

	float amount = 0.0034f * Vars.ragebot_slowwalk_amt/*options.misc.slow_walk_amount*/; //max 100

	Vector velocity = Globals::LocalPlayer->m_vecVelocity();
	QAngle direction;

	Math::VectorAngles(velocity, direction);

	float speed = velocity.Length2D();

	direction.yaw = cmd->viewangles.yaw - direction.yaw;

	Vector forward;

	Math::AngleVectors(direction, forward);

	Vector source = forward * -speed;

	if (speed >= (weapon_handle->GetCSWeaponData()->flMaxPlayerSpeed * amount))
	{
		cmd->forwardmove = source.x;
		cmd->sidemove = source.y;

	}
}

void CMisc::ClanTag()
{
	if (!Utils::IsInGame())
		return;

	if (!Vars.misc_clantag)
		return;

	static size_t lastTime = 0;

	if (GetTickCount() > lastTime)
	{
		clantag_ += clantag_.at(0);
		clantag_.erase(0, 1);

		Utils::SetClantag(clantag_.c_str());

		lastTime = GetTickCount() + 650;
	}
}

template<class T, class U>
T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

void CMisc::AutoStrafe(CUserCmd* cmd)
{
	if (!Vars.misc_autostrafe) return;
	if (!Utils::IsInGame()) return;
	if (!Globals::LocalPlayer) return;
	if (!Globals::LocalPlayer->IsAlive()) return;
	if (Globals::LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP) return;
	if (Globals::LocalPlayer->m_nMoveType() == MOVETYPE_LADDER) return;

	if (!(Globals::LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		if (cmd->mousedx > 1 || cmd->mousedx < -1)
		{
			cmd->sidemove = clamp(cmd->mousedx < 0.f ? -400.f : 400.f, -400, 400);
		}
		else
		{
			if (Globals::LocalPlayer->m_vecVelocity().Length2D() == 0 || Globals::LocalPlayer->m_vecVelocity().Length2D() == NAN || Globals::LocalPlayer->m_vecVelocity().Length2D() == INFINITE)
			{
				cmd->forwardmove = 400;
				return;
			}
			cmd->forwardmove = clamp(5850.f / Globals::LocalPlayer->m_vecVelocity().Length2D(), -400, 400);
			if (cmd->forwardmove < -400 || cmd->forwardmove > 400)
				cmd->forwardmove = 0;
			cmd->sidemove = clamp((cmd->command_number % 2) == 0 ? -400.f : 400.f, -400, 400);
			if (cmd->sidemove < -400 || cmd->sidemove > 400)
				cmd->sidemove = 0;
		}
	}
}

void CMisc::OverrideView(CViewSetup* vsView)
{
	if (!Globals::LocalPlayer)
		return;

	if (Globals::LocalPlayer->m_bIsScoped() && !Vars.misc_overridefov_inscope)
		return;

	if (Vars.misc_overridefov != 0)
		vsView->fov = (float)Vars.misc_overridefov;
}

void CMisc::FastDuck(CUserCmd* cmd)
{
	if (Vars.misc_fastduck)
		cmd->buttons |= IN_BULLRUSH;
}

void CMisc::Bhop(CUserCmd* cmd)
{
	if (!Vars.misc_bhop)
		return;

	static bool jumped_last_tick = false;
	static bool should_fake_jump = false;

	if (!jumped_last_tick && should_fake_jump) {
		should_fake_jump = false;
		cmd->buttons |= IN_JUMP;
	}
	else if (cmd->buttons & IN_JUMP) {
		if (Globals::LocalPlayer->m_fFlags() & FL_ONGROUND) {
			jumped_last_tick = true;
			should_fake_jump = true;
		}
		else {
			cmd->buttons &= ~IN_JUMP;
			jumped_last_tick = false;
		}
	}
	else {
		jumped_last_tick = false;
		should_fake_jump = false;
	}
}

bool CMisc::AntiKick(Hooks::tDispatchUserMessage oDispatchUserMessage, void* ecx, int type, unsigned int a3, unsigned int length, const void *msg_data)
{
	if (!Vars.misc_antikick)
		return false;
/*#ifndef _DEBUG
	if (type == CS_UM_VoteStart)
	{
		CCSUsrMsg_VoteStart Message;
		if (!Message.ParsePartialFromArray(msg_data, length))
			return oDispatchUserMessage(ecx, type, a3, length, msg_data);

		if (Message.team() == -1)
			return oDispatchUserMessage(ecx, type, a3, length, msg_data);

		player_info_t local_info;
		if (!Interfaces::Engine->GetPlayerInfo(Interfaces::Engine->GetLocalPlayer(), &local_info))
			return oDispatchUserMessage(ecx, type, a3, length, msg_data);

		if (Message.details_str().find(local_info.szName) != std::string::npos)
		{
			Interfaces::Engine->ExecuteClientCmd("callvote swapteams ");
		}
	}
#endif*/
}

void CMisc::AutoAccept(const char* pSoundEntry)
{
	if (!Vars.misc_autoaccept)
		return;

	if (!strcmp(pSoundEntry, XorStr("UIPanorama.popup_accept_match_beep"))) {
		static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), XorStr("55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12")));

		if (fnAccept) {

			fnAccept("");

			FLASHWINFO fi;
			fi.cbSize = sizeof(FLASHWINFO);
			fi.hwnd = InputSys::Get().GetMainWindow();
			fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
			fi.uCount = 0;
			fi.dwTimeout = 0;
			FlashWindowEx(&fi);
		}
	}
}
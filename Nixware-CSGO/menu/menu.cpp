#include <chrono>
#include <D3dx9tex.h>
#include "menu.hpp"
#include "../bass.h"
#include "../arrays.h"

#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/input.hpp"
#include "../options.hpp"
#include "ui.hpp"
#include "../fonts.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/directx9/imgui_impl_dx9.h"
#include "../features/misc/misc.hpp"
#include "../features/skinchanger/skins.hpp"
#include "../features/visuals/nightmode.hpp"
#include "../helpers/config_manager.hpp"
#include "Header.h"

#pragma comment(lib, "bass.lib")

HSTREAM stream;
ImFont* bigFont = nullptr;
ImFont* forTabs = nullptr;
ImFont* forTabs_text = nullptr;
ImFont* forTabs_text_x2 = nullptr;

ImFont* globalFont = nullptr;
WNDPROC original_proc = nullptr;
static int tabselected = 0;
IMGUI_API LRESULT   ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool isSkeet = false;

#define AIMBOT XorStr("A")
#define RAGE XorStr("K")
#define VISUALS XorStr("D")
#define SKINS XorStr("B")
#define COLORS XorStr("H")
#define MISC XorStr("G")

static int a1 = 0;

void CMenu::ImColors(ImGuiStyle& style)
{
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.59f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
	style.Colors[ImGuiCol_Separator] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.045f, 0.045f, 0.045f, 1.00f);

}

void DefaultColor()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

void CMenu::ImStyles(ImGuiStyle& style)
{//
	style.Alpha = 1.0f;             // Global alpha applies to everything in ImGui
	style.WindowPadding = ImVec2(8, 8);      // Padding within a window
	style.WindowRounding = 0.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);// Alignment for title bar text
	style.FramePadding = ImVec2(4, 1);      // Padding within a framed rectangle (used by most widgets)
	style.FrameRounding = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
	style.ItemSpacing = ImVec2(8, 4);      // Horizontal and vertical spacing between widgets/lines
	style.ItemInnerSpacing = ImVec2(4, 4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
	style.TouchExtraPadding = ImVec2(0, 0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
	style.IndentSpacing = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
	style.ColumnsMinSpacing = 6.0f;             // Minimum horizontal spacing between two columns
	style.ScrollbarSize = 10.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
	style.ScrollbarRounding = 3.0f;             // Radius of grab corners rounding for scrollbar
	style.GrabMinSize = 10.0f;            // Minimum width/height of a grab box for slider/scrollbar
	style.GrabRounding = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);// Alignment of button text when button is larger than text.
	style.DisplayWindowPadding = ImVec2(22, 22);    // Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
	style.DisplaySafeAreaPadding = ImVec2(4, 4);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
	style.AntiAliasedLines = true;             // Enable anti-aliasing on lines/borders. Disable if you are really short on CPU/GPU.
	style.CurveTessellationTol = 1.25f;            // Tessellation tolerance. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
}

void CMenu::Shutdown()
{
	Globals::PlayerListOpened = false;
	Globals::RadioOpened = false;
	Globals::MenuOpened = false;

	BASS_ChannelStop(stream);
	SetWindowLongPtrA(FindWindowA(XorStr("Valve001"), nullptr), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_proc));

	ImGui_ImplDX9_Shutdown();
}

LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (Globals::MenuOpened && ImGui_ImplDX9_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	return CallWindowProc(original_proc, hwnd, msg, wParam, lParam);
}

void CMenu::RadioFrame()
{
	if (!Globals::MenuOpened)
		return;

	if (!Globals::RadioOpened)
		return;

	ImGui::SetNextWindowSize(ImVec2(316, 118));
	if (ImGui::BeginAndroid(XorStr("Radio"), &Globals::RadioOpened, false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x + 158, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 2), Vars.barColor1.x, Vars.barColor1.y, Vars.barColor1.h, Vars.barColor1.w);
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x - 5, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y + 2), Vars.barColor2.x, Vars.barColor2.y, Vars.barColor2.h, Vars.barColor2.w);


		ImGui::Spacing();
		/*std::string songname;
		if (ImGui::Button("GetSong")) {
			std::string text = "Current track: ";
			std::string song = GetHardstyleSong().c_str();
			std::string copied = ", copied to clipboard.";

			g_Logger.WriteToLog((text + song + copied).c_str());
		}*/
		static bool radioInit = false;
		static int govno = 0;
		if (govno != Vars.radio_selected)
			radioInit = false;
		if (!radioInit) {
			BASS_Init(-1, 44100, BASS_DEVICE_3D, 0, NULL);
			stream = BASS_StreamCreateURL(stations[Vars.radio_selected].c_str(), 0, 0, NULL, 0);
			radioInit = true;
			Vars.radio_paused = false;
			govno = Vars.radio_selected;
		}

		static float vol = 0.1f;
		BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, vol);

		if (!Vars.radio_paused)
		{
			BASS_ChannelPlay(stream, false);
		}

		ImGui::PushItemWidth(300);

		if (ImGui::Combo(XorStr("##stancs"), &Vars.radio_selected, radio_name, IM_ARRAYSIZE(radio_name)))
		{
			BASS_ChannelStop(stream);
			stream = NULL;
			stream = BASS_StreamCreateURL(stations[Vars.radio_selected].c_str(), 0, 0, NULL, 0);
		}

		//ImGui::PushFont(awesomefont);

		if (ImGui::Button(XorStr("<<"), ImVec2(76, 32)))
		{
			if (Vars.radio_selected == 0)
				Vars.radio_selected = ARRAYSIZE(stations);

			Vars.radio_selected--;
			BASS_ChannelPause(stream);
			stream = NULL;
			stream = BASS_StreamCreateURL(stations[Vars.radio_selected].c_str(), 0, 0, NULL, 0);
		}
		ImGui::SameLine();

		char* nazvanie;

		if (!Vars.radio_paused)
			nazvanie = XorStr("||");
		else
			nazvanie = XorStr(">");

		if (ImGui::Button(nazvanie, ImVec2(132, 32)))
		{
			Vars.radio_paused = !Vars.radio_paused;
			BASS_ChannelPause(stream);
		}

		ImGui::SameLine();
		if (ImGui::Button(">>", ImVec2(76, 32))) {
			Vars.radio_selected++;
			if (Vars.radio_selected > ARRAYSIZE(stations) - 1)
				Vars.radio_selected = 0;

			BASS_ChannelPause(stream);
			stream = NULL;
			stream = BASS_StreamCreateURL(stations[Vars.radio_selected].c_str(), 0, 0, NULL, 0);
		}
		ImGui::Spacing();

		ImGui::SliderFloat(XorStr("##Volume"), &vol, 0.f, 1.f);
	}
	ImGui::End();
}

void CMenu::PlayerListFrame()
{
	static int currentPlayer = -1;

	if (!Utils::IsInGame())
		currentPlayer = -1;

	if (!Globals::MenuOpened)
		return;

	if (!Globals::PlayerListOpened)
		return;

	ImGui::SetNextWindowSize(ImVec2(500, 300));
	if (ImGui::BeginAndroid(XorStr("plist1337"), &Globals::PlayerListOpened, false, ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth() / 2, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 2), Vars.barColor1.x, Vars.barColor1.y, Vars.barColor1.h, Vars.barColor1.w);
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x - 5, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y + 2), Vars.barColor2.x, Vars.barColor2.y, Vars.barColor2.h, Vars.barColor2.w);
		ImGui::Spacing();


		/*if (Utils::IsInGame())
		{
			for (int i = 0; i <= Interfaces::GlobalVars->maxClients; i++)
			{
				if (i == Interfaces::Engine->GetLocalPlayer())
					continue;

				C_BasePlayer* Target = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(i);
				if (!Target)
					continue;
				if (Target->GetClientClass()->m_ClassID != CCSPlayer)
					continue;

				if (Target->IsTeammate())
					continue;

				if (Target->GetPlayerInfo().szName == "GOTV" && !Target->m_hActiveWeapon())
					continue;

				player_info_t entityInformation;
				Interfaces::Engine->GetPlayerInfo(i, &entityInformation);

				player_info_t localinfo; Interfaces::Engine->GetPlayerInfo(Interfaces::Engine->GetLocalPlayer(), &localinfo);

				char buf[255]; sprintf_s(buf, u8"%s", entityInformation.szName);
				ImGui::Text(buf);
			}
		}*/
	}
	ImGui::End();
}

void CMenu::RenderLegitbotTab()
{
	static int selected = 0;

	if (ImGui::ButtonSelectable(XorStr("Globals"), ImVec2(220, 40), selected == 0, forTabs_text, forTabs_text_x2))
		selected = 0;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(ActiveWeaponName.c_str(), ImVec2(220, 40), selected == 1, forTabs_text, forTabs_text_x2))
		selected = 1;

	switch (selected)
	{
	case 0:
		ImGui::BeginGroup();
		ImGui::Spacing();
		ImGui::Checkbox(XorStr("Enable Legitbot"), &Vars.legit_enable);
		ImGui::Checkbox(XorStr("Ignore Team##AIM"), &Vars.legit_ignoreteam);
		ImGui::Checkbox(XorStr("Smoke Check"), &Vars.legit_smokecheck);
		ImGui::Checkbox(XorStr("Flash Check"), &Vars.legit_flashcheck);
		ImGui::Checkbox(XorStr("Draw FOV"), &Vars.legit_drawfovaimbot);
		ImGui::Checkbox(XorStr("Backtracking"), &Vars.legit_backtracking);

		ImGui::PushItemWidth(142);
		ImGui::Combo(XorStr("Triggerbot key"), &Vars.legit_trigger_key, AllKeys, IM_ARRAYSIZE(AllKeys));
		ImGui::PopItemWidth();

		//ImGui::SliderFloat("test1", &Vars.test, 200, 500);
		//ImGui::SliderFloat("test2", &Vars.test2, 200, 500);
		//ImGui::SliderFloat("test3", &Vars.test3, 0.f, 30.f);

		ImGui::EndGroup();
		break;
	case 1:
		if (!Globals::WeaponTabValid)
			ImGui::Text(XorStr("Take a gun"));
		else
		{
			ImGui::PushItemWidth(142);
			ImGui::BeginGroup();
			ImGui::Spacing();
			ImGui::Checkbox(XorStr("Enable aimbot"), &WeaponSettings->aim_enabled);
			if (Globals::LocalPlayer->m_hActiveWeapon()->IsPistol()) {
				ImGui::Checkbox(XorStr("Autopistol"), &WeaponSettings->aim_autopistol);
			}
			if (Globals::LocalPlayer->m_hActiveWeapon()->IsSniper()) {
				ImGui::Checkbox(XorStr("Only on scope"), &WeaponSettings->aim_only_in_zoom);
			}
			ImGui::Checkbox(XorStr("Nearest"), &WeaponSettings->aim_nearest);
			ImGui::Text(XorStr("Hitbox:"));
			ImGui::Combo(XorStr("##aimbot.hitbox"), &WeaponSettings->aim_hitbox, hitbox_list, IM_ARRAYSIZE(hitbox_list));
			ImGui::Text(XorStr("Hitchance"));
			ImGui::SliderFloat(XorStr("##hitchnc"), &WeaponSettings->aim_hitchance, 0.f, 100.f, "%.2f %%");
			ImGui::Checkbox(XorStr("Ignore walls"), &WeaponSettings->aim_ignore_walls);
			ImGui::Checkbox(XorStr("pSilent"), &WeaponSettings->aim_perfect_silent);
			ImGui::Checkbox(XorStr("Recoil based fov"), &WeaponSettings->aim_recoil_based_fov);

			//ImGui::Checkbox("Dynamic fov", &WeaponSettings->fov_type);

			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::Text(XorStr(WeaponSettings->aim_perfect_silent ? "FOV - pSilent" : "FOV"));
			if (WeaponSettings->aim_perfect_silent)
				ImGui::SliderFloat(XorStr("##silent_fov"), &WeaponSettings->aim_psilent_fov, 0, 3, "%.2f");
			else
				ImGui::SliderFloat(XorStr("##FOV"), &WeaponSettings->aim_fov, 0, 50, "%.2f");

			ImGui::Text(WeaponSettings->aim_perfect_silent ? XorStr("Smooth (Inactive)") : XorStr("Smooth"));
			ImGui::SliderFloat(XorStr("##Smooth"), &WeaponSettings->aim_smooth, 0, 20, "%.2f");

			if (Globals::LocalPlayer->m_hActiveWeapon()->IsAutomaticGun()) {
				ImGui::Text(XorStr("FOV after 1 bullet"));
				ImGui::SliderFloat(XorStr("##FOV - 2 b"), &WeaponSettings->aim_fov_2, 0, 50, "%.2f");
				ImGui::Text(XorStr("Smooth after 1 bullet"));
				ImGui::SliderFloat(XorStr("##Smooth 2 b"), &WeaponSettings->aim_smooth_2, 0, 20, "%.2f");
			}
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::Spacing();
			ImGui::Checkbox(XorStr("RCS"), &WeaponSettings->aim_rcs);
			ImGui::SliderInt(XorStr("##aimbot.rcs_x"), &WeaponSettings->aim_rcs_x, 0, 100, "X: %.0f");
			ImGui::SliderInt(XorStr("##aimbot.rcs_y"), &WeaponSettings->aim_rcs_y, 0, 100, "Y: %.0f");

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Text(XorStr("ShotDelay"));
			ImGui::SliderInt(XorStr("##aimbot.shot_delay"), &WeaponSettings->aim_shot_delay, 0, 100);

			ImGui::Text(XorStr("KillDelay"));
			ImGui::SliderInt(XorStr("##aimbot.kill_delay"), &WeaponSettings->aim_kill_delay, 0, 1000);

			ImGui::EndGroup();

			ImGui::Spacing();
			const ImRect bb(ImVec2(ImGui::GetCurrentWindow()->Pos.x, ImGui::GetCurrentWindow()->DC.CursorPos.y), ImVec2(ImGui::GetCurrentWindow()->Pos.x + ImGui::GetCurrentWindow()->Size.x, ImGui::GetCurrentWindow()->DC.CursorPos.y + 1.0f));
			ImGui::GetCurrentWindow()->DrawList->AddLine(bb.Min + ImVec2(132, 0), ImVec2(bb.Max.x, bb.Min.y), ImGui::GetColorU32(ImGuiCol_Separator));
			ImGui::Spacing();


			ImGui::BeginGroup();

			ImGui::BeginGroup();
			ImGui::Spacing();
			ImGui::Checkbox(XorStr("Enable triggerbot"), &WeaponSettings->trigger_enable);
			ImGui::Checkbox(XorStr("Head"), &WeaponSettings->trigger_Head);
			ImGui::Checkbox(XorStr("Body"), &WeaponSettings->trigger_Body);
			ImGui::Checkbox(XorStr("Misc"), &WeaponSettings->trigger_Misc);
			ImGui::EndGroup();

			ImGui::SameLine(150);

			ImGui::BeginGroup();
			ImGui::Text(XorStr("Delay"));
			ImGui::SliderInt(XorStr("##Delay"), &WeaponSettings->trigger_delay, 0, 100);
			ImGui::Text(XorStr("HitChance"));
			ImGui::SliderFloat(XorStr("##hitchnctr"), &WeaponSettings->trigger_hitchance, 0.f, 100.f, "%.2f %%");
			ImGui::EndGroup();
			ImGui::EndGroup();

			ImGui::PopItemWidth();
		}
		break;
	}
}

void CMenu::RenderSkinsTab()
{
	static int selected = 0;

	if (ImGui::ButtonSelectable(XorStr("Knife CT"), ImVec2(100, 40), selected == 0, forTabs_text, forTabs_text_x2))
		selected = 0;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Knife TT"), ImVec2(100, 40), selected == 1, forTabs_text, forTabs_text_x2))
		selected = 1;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Gloves"), ImVec2(100, 40), selected == 2, forTabs_text, forTabs_text_x2))
		selected = 2;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(ActiveWeaponName.c_str(), ImVec2(100, 40), selected == 3, forTabs_text, forTabs_text_x2))
		selected = 3;

	switch (selected)
	{
	case 0:
		ImGui::Checkbox(XorStr("Enabled"), &Vars.weapon[WEAPON_KNIFE].enabled);
		ImGui::InputInt(XorStr("Seed"), &Vars.weapon[WEAPON_KNIFE].seed);
		ImGui::InputInt(XorStr("StatTrak"), &Vars.weapon[WEAPON_KNIFE].stat_trak);
		ImGui::SliderFloat(XorStr("Wear"), &Vars.weapon[WEAPON_KNIFE].wear, 0.f, 1.f, "%.3f");
		ImGui::InputText(XorStr("Name Tag"), Vars.weapon[WEAPON_KNIFE].custom_name, 32);

		ImGui::Combo(XorStr("Paint Kit"), &Vars.weapon[WEAPON_KNIFE].paint_kit_vector_index, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_skins[idx].name.c_str();
			return true;
		}, nullptr, k_skins.size(), 10);

		ImGui::Combo(XorStr("Knife##Model"), &Vars.weapon[WEAPON_KNIFE].definition_override_vector_index, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_knife_names.at(idx).name;
			return true;
		}, nullptr, k_knife_names.size(), 5);
		break;
	case 1:
		ImGui::Checkbox("Enabled", &Vars.weapon[WEAPON_KNIFE_T].enabled);
		ImGui::InputInt("Seed", &Vars.weapon[WEAPON_KNIFE_T].seed);
		ImGui::InputInt("StatTrak", &Vars.weapon[WEAPON_KNIFE_T].stat_trak);
		ImGui::SliderFloat("Wear", &Vars.weapon[WEAPON_KNIFE_T].wear, 0.f, 1.f, "%.3f");
		ImGui::InputText("Name Tag", Vars.weapon[WEAPON_KNIFE_T].custom_name, 32);

		ImGui::Combo("Paint Kit", &Vars.weapon[WEAPON_KNIFE_T].paint_kit_vector_index, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_skins[idx].name.c_str();
			return true;
		}, nullptr, k_skins.size(), 10);

		ImGui::Combo("Knife##Model", &Vars.weapon[WEAPON_KNIFE_T].definition_override_vector_index, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_knife_names.at(idx).name;
			return true;
		}, nullptr, k_knife_names.size(), 5);
		break;
	case 2:
		ImGui::Checkbox(XorStr("Enabled"), &Vars.weapon[5028].enabled);
		ImGui::Combo(XorStr("Glove##model"), &Vars.weapon[5028].definition_override_vector_index, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_glove_names.at(idx).name;
			return true;
		}, nullptr, k_glove_names.size(), 5);

		ImGui::Combo(XorStr("Paint Kit"), &Vars.weapon[5028].paint_kit_vector_index, [](void* data, int idx, const char** out_text)
		{
			*out_text = k_gloves[idx].name.c_str();
			return true;
		}, nullptr, k_gloves.size(), 10);
		ImGui::SliderFloat(XorStr("Wear"), &Vars.weapon[5028].wear, 0.f, 1.f, "%.3f");

		break;
	case 3:
		if (!Globals::WeaponTabValid)
			ImGui::Text(XorStr("Take a gun"));
		else {
			ImGui::Checkbox(XorStr("Enabled"), &WeaponSettings->enabled);
			ImGui::InputInt(XorStr("Seed"), &WeaponSettings->seed);
			ImGui::InputInt(XorStr("StatTrak"), &WeaponSettings->stat_trak);
			ImGui::SliderFloat(XorStr("Wear"), &WeaponSettings->wear, 0.f, 1.f, "%.3f");
			ImGui::InputText(XorStr("Name Tag"), WeaponSettings->custom_name, 32);

			ImGui::Combo(XorStr("Paint Kit"), &WeaponSettings->paint_kit_vector_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = k_skins[idx].name.c_str();
				return true;
			}, nullptr, k_skins.size(), 10);
		}
		break;
	}

	if (ImGui::Button(XorStr("Update")))
	{
		CSkinChanger::Get().Update();
	}
}

void CMenu::RenderRagebotTab()
{
	static int selected = 0;

	if (ImGui::ButtonSelectable(XorStr("Globals"), ImVec2(220, 40), selected == 0, forTabs_text, forTabs_text_x2))
		selected = 0;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Weapon"), ImVec2(220, 40), selected == 1, forTabs_text, forTabs_text_x2))
		selected = 1;

	if (selected == 0) 
	{
		ImGui::BeginGroup();
		{
			ImGui::PushItemWidth(152);
			ImGui::Checkbox(XorStr("Enable Ragebot"), &Vars.ragebot_enabled);
			ImGui::Checkbox(XorStr("Auto zeus"), &Vars.ragebot_autozeus);
			ImGui::Text(XorStr("FOV"));
			ImGui::SliderInt(XorStr("##FOV"), &Vars.ragebot_fov, 0, 180);
			ImGui::PopItemWidth();
		}
		ImGui::EndGroup();

		ImGui::SameLine(162);

		ImGui::BeginGroup();
		{
			ImGui::PushItemWidth(152);
			ImGui::SliderInt("Fakelag", &Vars.ragebot_fakelag_amt, 0, 16);
			ImGui::SliderInt("Slowwalk", &Vars.ragebot_slowwalk_amt, 0, 100);
			ImGui::Combo(XorStr("Slowwalk key"), &Vars.ragebot_slowwalk_key, AllKeys, IM_ARRAYSIZE(AllKeys));
			ImGui::Spacing();
			ImGui::Combo("Pitch", &Vars.ragebot_antiaim_pitch, aa_pitch_list, IM_ARRAYSIZE(aa_pitch_list));
			ImGui::Combo("Yaw", &Vars.ragebot_antiaim_yaw, aa_yaw_list, IM_ARRAYSIZE(aa_yaw_list));
			ImGui::Checkbox("Desync", &Vars.ragebot_antiaim_desync);
			ImGui::PopItemWidth();
		}
		ImGui::EndGroup();

	}
	else if (selected == 1) {
		static int curGroup = 0;
		if (ImGui::ButtonSelectable("Pistols", ImVec2(55, 25), curGroup == WEAPON_GROUPS::PISTOLS)) curGroup = WEAPON_GROUPS::PISTOLS;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("Rifles", ImVec2(55, 25), curGroup == WEAPON_GROUPS::RIFLES)) curGroup = WEAPON_GROUPS::RIFLES;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("SMG", ImVec2(55, 25), curGroup == WEAPON_GROUPS::SMG)) curGroup = WEAPON_GROUPS::SMG;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("Shotguns", ImVec2(55, 25), curGroup == WEAPON_GROUPS::SHOTGUNS)) curGroup = WEAPON_GROUPS::SHOTGUNS;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("Scout", ImVec2(55, 25), curGroup == WEAPON_GROUPS::SCOUT)) curGroup = WEAPON_GROUPS::SCOUT;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("Auto", ImVec2(55, 25), curGroup == WEAPON_GROUPS::AUTO)) curGroup = WEAPON_GROUPS::AUTO;
		ImGui::SameLine();
		if (ImGui::ButtonSelectable("AWP", ImVec2(55, 25), curGroup == WEAPON_GROUPS::AWP)) curGroup = WEAPON_GROUPS::AWP;
		ImGui::BeginGroup();
		{
			ImGui::PushItemWidth(142);

			ImGui::Checkbox(XorStr("Auto Scope"), &Vars.ragebot_autoscope[curGroup]);
			ImGui::Checkbox(XorStr("Auto Stop"), &Vars.ragebot_autostop[curGroup]);
			ImGui::Checkbox(XorStr("Auto Crouch"), &Vars.ragebot_autocrouch[curGroup]);

			ImGui::SliderFloat(XorStr("Hitchance"), &Vars.ragebot_hitchance[curGroup], 0.f, 100.f);
			ImGui::SliderFloat(XorStr("Min Damage"), &Vars.ragebot_mindamage[curGroup], 0.f, 100.f);
			ImGui::SliderInt(XorStr("Baim after x shots"), &Vars.ragebot_baim_after_shots[curGroup], 0, 10);

			ImGui::Combo("Target Selection", &Vars.ragebot_selection[curGroup], selectiontypes, IM_ARRAYSIZE(selectiontypes));

			static std::string prevValue = "Select";
			if (ImGui::BeginCombo("Hitscan", "Select", 0))
			{
				//prevValue = "Hitscan";
				std::vector<std::string> vec;
				for (size_t i = 0; i < IM_ARRAYSIZE(hitboxes); i++)
				{
					ImGui::Selectable(hitboxes[i], &Vars.ragebot_hitbox[i][curGroup], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
					if (Vars.ragebot_hitbox[i][curGroup])
						vec.push_back(hitboxes[i]);
				}

				for (size_t i = 0; i < vec.size(); i++)
				{
					if (vec.size() == 1)
						prevValue += vec.at(i);
					else if (i != vec.size())
						prevValue += vec.at(i) + ", ";
					else
						prevValue += vec.at(i);
				}
				ImGui::EndCombo();
			} 
			ImGui::PopItemWidth();
		}
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		{
			ImGui::PushItemWidth(142);
			ImGui::Text("Multipoint Scale");
			for (int i = 0; i < 8; i++) {
				if (Vars.ragebot_hitbox[i])
					ImGui::SliderFloat(hitboxes[i], &Vars.ragebot_hitbox_multipoint_scale[i][curGroup], 0.f, 1.f);
			}
			ImGui::PopItemWidth();
		}
		ImGui::EndGroup();
	}
}

void CMenu::RenderVisualsTab()
{
	static int selected = 0;

	if (ImGui::ButtonSelectable(XorStr("Players"), ImVec2(135, 40), selected == 0, forTabs_text, forTabs_text_x2))
		selected = 0;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Misc##Visuals_tab"), ImVec2(135, 40), selected == 1, forTabs_text, forTabs_text_x2))
		selected = 1;
	ImGui::SameLine();
	if (ImGui::ButtonSelectable(XorStr("Colors##Visuals_tab"), ImVec2(135, 40), selected == 2, forTabs_text, forTabs_text_x2))
		selected = 2;

	switch (selected)
	{
	case 0:
		ImGui::BeginGroup();
		ImGui::Checkbox(XorStr("Enable esp"), &Vars.esp_enabled);
		ImGui::Checkbox(XorStr("Ignore team"), &Vars.esp_ignore_team);
		ImGui::Checkbox(XorStr("Visible only"), &Vars.esp_visible_only);

		ImGui::Spacing();
		ImGui::Checkbox(XorStr("Box"), &Vars.esp_player_boxes);
		ImGui::Checkbox(XorStr("Health"), &Vars.esp_player_health);
		ImGui::Checkbox(XorStr("Name"), &Vars.esp_player_names);
		ImGui::Checkbox(XorStr("Weapon"), &Vars.esp_player_weapons);

		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Checkbox(XorStr("Enable chams##CHAMS"), &Vars.chams_player_enabled);
		ImGui::Checkbox(XorStr("Ignore team##CHAMS"), &Vars.chams_player_ignore_team);
		ImGui::Checkbox(XorStr("Visible only##CHAMS"), &Vars.chams_player_visible_only);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Checkbox(XorStr("Enable glow##GLOW"), &Vars.glow_enabled);
		ImGui::Checkbox(XorStr("Ignore team##GLOW"), &Vars.glow_ignore_team);
		ImGui::Checkbox(XorStr("Visible only##GLOW"), &Vars.glow_visible_only);
		ImGui::EndGroup();
		break;
	case 1:
		ImGui::BeginGroup();

		ImGui::Text(XorStr("Item ESP"));
		ImGui::Checkbox(XorStr("Enable"), &Vars.esp_drop_enable);
		ImGui::PushItemWidth(150);
		ImGui::Text(XorStr("Distance"));
		ImGui::SliderInt(XorStr("##Distance"), &Vars.esp_drop_distance, 0, 3500);
		ImGui::PopItemWidth();
		ImGui::BeginChild(XorStr("##itemesp"), ImVec2(150, 200));
		ImGui::Checkbox(XorStr("Dropped Weapons"), &Vars.esp_dropped_weapons);
		ImGui::Checkbox(XorStr("Planted C4"), &Vars.esp_planted_c4);
		ImGui::Checkbox(XorStr("Pistol Case"), &Vars.esp_case_pistol);
		ImGui::Checkbox(XorStr("Light Case"), &Vars.esp_case_light_weapon);
		ImGui::Checkbox(XorStr("Heavy Case"), &Vars.esp_case_heavy_weapon);
		ImGui::Checkbox(XorStr("Explosive Case"), &Vars.esp_case_explosive);
		ImGui::Checkbox(XorStr("Tools case"), &Vars.esp_case_tools);
		ImGui::Checkbox(XorStr("Airdrop"), &Vars.esp_random);
		ImGui::Checkbox(XorStr("Full Armor"), &Vars.esp_dz_armor_helmet);
		ImGui::Checkbox(XorStr("Helmet"), &Vars.esp_dz_helmet);
		ImGui::Checkbox(XorStr("Armor"), &Vars.esp_dz_armor);
		ImGui::Checkbox(XorStr("Tablet Upgrade"), &Vars.esp_upgrade_tablet);
		ImGui::Checkbox(XorStr("Briefcase"), &Vars.esp_briefcase);
		ImGui::Checkbox(XorStr("Parachute"), &Vars.esp_parachutepack);
		ImGui::Checkbox(XorStr("Cash Dufflebag"), &Vars.esp_dufflebag);
		ImGui::Checkbox(XorStr("Ammobox"), &Vars.esp_ammobox);
		ImGui::EndChild();
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Text(XorStr("Misc visuals"));
		ImGui::BeginChild(XorStr("##othervisuals"), ImVec2(150, 200));
		ImGui::Checkbox(XorStr("Spread Xair"), &Vars.visuals_spreadxair);
		ImGui::Checkbox(XorStr("Crosshair"), &Vars.visuals_crosshair);
		ImGui::Checkbox(XorStr("No visual recoil"), &Vars.visuals_norecoil);
		ImGui::Checkbox(XorStr("No flash"), &Vars.visuals_noflash);

		if (ImGui::Checkbox(XorStr("Nightmode"), &Vars.visuals_nightmode))
			CNightmode::Get().PerformNightmode();

		ImGui::Checkbox(XorStr("Remove scope borders"), &Vars.visuals_noscopeborder);
		ImGui::Checkbox(XorStr("Thirdperson"), &Vars.visuals_thirdperson); ImGui::SameLine();
		ImGui::PushItemWidth(60);
		ImGui::Combo(XorStr("##key"), &Vars.visuals_thirdperson_key, AllKeys, IM_ARRAYSIZE(AllKeys));
		ImGui::PopItemWidth();
		ImGui::Checkbox(XorStr("Watermark"), &Vars.visuals_watermark);

		ImGui::EndChild();

		ImGui::EndGroup();
		break;
	case 2:
		ImGui::BeginGroup();
		ImGui::Text(XorStr("Esp"));
		ImGui::ColorEdit4(XorStr("Enemy visible##esp"), Vars.color_esp_enemy_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Enemy hidden##esp"), Vars.color_esp_enemy_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team visible##esp"), Vars.color_esp_team_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team hidden##esp"), Vars.color_esp_team_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Text(XorStr("Chams"));
		ImGui::ColorEdit4(XorStr("Enemy visible##chams"), Vars.color_chams_enemy_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Enemy hidden##chams"), Vars.color_chams_enemy_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team visible##chams"), Vars.color_chams_team_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team hidden##chams"), Vars.color_chams_team_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Text(XorStr("Glow"));
		ImGui::ColorEdit4(XorStr("Enemy visible##glow"), Vars.color_glow_enemy_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Enemy hidden##glow"), Vars.color_glow_enemy_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team visible##glow"), Vars.color_glow_team_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit4(XorStr("Team hidden##glow"), Vars.color_glow_team_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::EndGroup();
		break;
	}
}

void CMenu::RenderMiscTab()
{
	ImGui::PushItemWidth(150);

	ImGui::Checkbox(XorStr("BunnyHop"), &Vars.misc_bhop);
	ImGui::Checkbox(XorStr("FastDuck"), &Vars.misc_fastduck);
	ImGui::Checkbox(XorStr("AutoStrafe"), &Vars.misc_autostrafe);
	ImGui::Checkbox(XorStr("Clantag"), &Vars.misc_clantag);
	ImGui::Checkbox(XorStr("AutoAccept"), &Vars.misc_autoaccept);
	//ImGui::Checkbox(XorStr("AntiKick"), &Vars.misc_antikick);
	ImGui::Checkbox(XorStr("Spectator List"), &Vars.misc_spectlist);
	ImGui::SliderInt(XorStr("Override fov"), &Vars.misc_overridefov, 0, 180);
	ImGui::Checkbox("Force in scope", &Vars.misc_overridefov_inscope);
	ImGui::SliderInt(XorStr("Viewmodel fov"), &Vars.misc_viewmodelfov, 0, 180);

	//ImGui::Checkbox(XorStr("Fake Latency"), &Vars.misc_latency_enable);
	//ImGui::PushItemWidth(150);
	//ImGui::SliderInt(XorStr("##latency_amt"), &Vars.misc_latency_amt, 0, 500);

}

void CMenu::MainFrame()
{
	if (!Globals::MenuOpened)
		return;

	ImGui::SetNextWindowSize(ImVec2(600, 381));
	if (ImGui::BeginAndroid(XorStr("nixwaree1337"), &Globals::MenuOpened, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 2), Vars.barColor1.x, Vars.barColor1.y, Vars.barColor1.h, Vars.barColor1.w);
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x - 5, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y + 2), Vars.barColor2.x, Vars.barColor2.y, Vars.barColor2.h, Vars.barColor2.w);
		ImGui::Spacing();
		ImGui::BeginChild("##tabs", ImVec2(125, 338));
		{
			ImGui::SameLine(18);
			ImGui::Dummy(ImVec2(0, 30 ));

			ImGui::Dummy(ImVec2(0, 0));

			ImGui::PushFont(forTabs_text);

			if (ImGui::MyButton(XorStr("Legitbot"), AIMBOT, forTabs, tabselected == 0, ImVec2(130, 40)))
				tabselected = 0;
			if (ImGui::MyButton(XorStr("Ragebot"), RAGE, forTabs, tabselected == 1, ImVec2(130, 40)))
				tabselected = 1;
			if (ImGui::MyButton(XorStr("Visuals"), VISUALS, forTabs, tabselected == 2, ImVec2(130, 40)))
				tabselected = 2;
			if (ImGui::MyButton(XorStr("Skins"), SKINS, forTabs, tabselected == 3, ImVec2(130, 40)))
				tabselected = 3;
			if (ImGui::MyButton(XorStr("Misc"), MISC, forTabs, tabselected == 4, ImVec2(130, 40)))
				tabselected = 4;
			ImGui::PopFont();
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			switch (tabselected)
			{
			case 0:
				RenderLegitbotTab();
				break;
			case 1: 
				RenderRagebotTab();
				break;
			case 2:
				RenderVisualsTab();
				break;
			case 3:
				RenderSkinsTab();
				break;
			case 4:
				RenderMiscTab();
				break;
			}
		}
		ImGui::EndGroup();

		ImGui::BeginGroup();
		/*if (ImGui::ButtonGradientEx(XorStr("Unload"), ImVec2(110, 24)))
			ImGui::OpenPopup(XorStr("Unload window"));

		bool open = true;
		if (ImGui::BeginPopupModal(XorStr("Unload window"), &open))
		{
			ImGui::Text(XorStr("Are you sure?"));
			ImGui::Spacing();
			if (ImGui::Button(XorStr("Unload")))
				g_Unload = true;
			ImGui::SameLine();
			if (ImGui::Button(XorStr("Cancel")))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::SameLine();*/
		
		ImGui::PushItemWidth(140);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.045f, 0.045f, 0.045f, 1.00f));

		ImGui::Combo(XorStr("##configselect"), &a1, items, ARRAYSIZE(items));
		ImGui::PopStyleColor();

		ImGui::PopItemWidth();

		ImGui::SameLine();

		if (ImGui::ButtonGradientEx(XorStr("Load"), ImVec2(140, 24)))
		{
			CConfig::Get().Load(items[a1]);
			CSkinChanger::Get().Update();
		}
		ImGui::SameLine();
		if (ImGui::ButtonGradientEx(XorStr("Save"), ImVec2(140, 24)))
		{
			CConfig::Get().Save(items[a1]);
		}
		ImGui::SameLine();
		/*if (ImGui::Button(XorStr("Rehook"), ImVec2(69, 23)))
		{
			Hooks::net_channel_hook_manager.~vfunc_hook();
			Hooks::net_channel_hook_manager.setup(reinterpret_cast<DWORD**>(Interfaces::ClientState->m_NetChannel));
			Hooks::oSendDatagram = Hooks::net_channel_hook_manager.get_original<Hooks::SendDatagram_t>(index::SendDatagram);
			Hooks::net_channel_hook_manager.hook_index(index::SendDatagram, Hooks::hkSendDatagram);
		}
		ImGui::SameLine();*/
		if (ImGui::ButtonGradientEx(XorStr("Radio"), ImVec2(140, 24)))
		{
			Globals::RadioOpened = !Globals::RadioOpened;
		}
		//ImGui::Checkbox(XorStr("PlayerList"), &CMenu::Get().PlayerListOpened);

		ImGui::EndGroup();
		ImGui::End();
	}
}

bool CMenu::Initialize(IDirect3DDevice9* device)
{
	static bool once = false;
	if (!once)
	{
		HWND hWindow = FindWindowA(XorStr("Valve001"), 0);

		ImGui_ImplDX9_Init(hWindow, device);
		ImGuiStyle& style = ImGui::GetStyle();

		ImStyles(ImGui::GetStyle());
		ImColors(ImGui::GetStyle());

		ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Ubuntu_compressed_data, Ubuntu_compressed_size, 13, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		forTabs = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(IconsForTabs_compressed_data, IconsForTabs_compressed_size, 25, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		forTabs_text = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Ubuntu_compressed_data, Ubuntu_compressed_size, 15, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		forTabs_text_x2 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Ubuntu_compressed_data, Ubuntu_compressed_size, 18, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

		if (anime_picture == nullptr)D3DXCreateTextureFromFileInMemoryEx(device
			, &anime, sizeof(anime),
			278, 378, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &anime_picture);

		ImGui_ImplDX9_CreateDeviceObjects();
		original_proc = (WNDPROC)SetWindowLongA(hWindow, GWL_WNDPROC, (LONG)(LONG_PTR)proc);

		ImFontConfig config;
		config.FontDataOwnedByAtlas = false;

		ImGui::GetIO().IniFilename = nullptr;
		once = true;
	}
	return once;
}

void CMenu::SpectatorListFrame()
{
	if (!Vars.misc_spectlist)
		return;

	int specs = 0;
	int modes = 0;
	std::string spect = "";
	std::string mode = "";
	int DrawIndex = 1;

	for (int playerId : Utils::GetObservervators(Interfaces::Engine->GetLocalPlayer()))
	{
		if (playerId == Interfaces::Engine->GetLocalPlayer())
			continue;

		C_BasePlayer* pPlayer = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(playerId);

		if (!pPlayer)
			continue;

		player_info_t Pinfo;
		Interfaces::Engine->GetPlayerInfo(playerId, &Pinfo);

		if (Pinfo.fakeplayer)
			continue;

		spect += Pinfo.szName;
		spect += "\n";
		specs++;

		if (spect != "")
		{
			Color PlayerObsColor;
			switch (pPlayer->m_iObserverMode())
			{
			case 4:
				mode += XorStr("Perspective");
				break;
			case 5:
				mode += XorStr("3rd Person");
				break;
			case 6:
				mode += XorStr("Free look");
				break;
			case 1:
				mode += XorStr("Deathcam");
				break;
			case 2:
				mode += XorStr("Freezecam");
				break;
			case 3:
				mode += XorStr("Fixed");
				break;
			default:
				break;
			}

			mode += "\n";
			modes++;
		}
	}
	bool misc_Spectators = true;

	if (ImGui::BeginAndroid(XorStr("Spectator List - Nixware"), &misc_Spectators, false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x + 100, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 2), Vars.barColor1.x, Vars.barColor1.y, Vars.barColor1.h, Vars.barColor1.w);
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(ImGui::GetWindowPos().x - 5, ImGui::GetWindowPos().y - 6), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y + 2), Vars.barColor2.x, Vars.barColor2.y, Vars.barColor2.h, Vars.barColor2.w);
		if (specs > 0) spect += "\n";     /*ËÀÑÒ*/
		if (modes > 0) mode += "\n";
		ImVec2 size = ImGui::CalcTextSize(spect.c_str());

		ImGui::SetWindowSize(ImVec2(200, 25 + size.y));
		ImGui::Columns(2);

		ImGui::Text(XorStr("Name"));
		ImGui::NextColumn();

		ImGui::Text(XorStr("Mode"));
		ImGui::NextColumn();
		ImGui::Separator();

		ImGui::Text(spect.c_str());
		ImGui::NextColumn();

		ImGui::Text(mode.c_str());
		ImGui::Columns(1);
		DrawIndex++;
	}
	ImGui::End();
}

void CMenu::EndScene(IDirect3DDevice9* device)
{	
	Globals::WeaponTabValid = Utils::IsInGame() && Globals::LocalPlayer->IsAlive() && Globals::LocalPlayer->m_hActiveWeapon()->IsGun();

	if (CMenu::Initialize(device))
	{
		if (Utils::IsKeyPressed(VK_INSERT))
			Globals::MenuOpened = !Globals::MenuOpened;

		if (isSkeet) {
			Vars.barColor1 = CVariables::Colour4(ImColor(201, 84, 192), ImColor(204, 227, 54), ImColor(204, 227, 54), ImColor(201, 84, 192));
			Vars.barColor2 = CVariables::Colour4(ImColor(55, 177, 218), ImColor(201, 84, 192), ImColor(201, 84, 192), ImColor(55, 177, 218));
		}
		else {
			Vars.barColor1 = CVariables::Colour4(ImColor(255, 0, 0), ImColor(255, 0, 0), ImColor(255, 0, 0), ImColor(255, 0, 0));
			Vars.barColor2 = CVariables::Colour4(ImColor(255, 0, 0), ImColor(255, 0, 0), ImColor(255, 0, 0), ImColor(255, 0, 0));
		}

		DWORD dwOld_D3DRS_COLORWRITEENABLE; IDirect3DVertexDeclaration9* vertDec; IDirect3DVertexShader9* vertShader;
		device->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		device->GetVertexDeclaration(&vertDec);
		device->GetVertexShader(&vertShader);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);
		ImGui_ImplDX9_NewFrame();

		CMenu::MainFrame();
		CMenu::RadioFrame();
		CMenu::PlayerListFrame();
		CMenu::SpectatorListFrame();

		ImGui::Render();
		device->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		device->SetVertexDeclaration(vertDec);
		device->SetVertexShader(vertShader);
	}
} 

void CMenu::InvalidateDeviceObjects() { ImGui_ImplDX9_InvalidateDeviceObjects(); }
void CMenu::CreateDeviceObjects() { ImGui_ImplDX9_CreateDeviceObjects(); }
#pragma once

#include <string>
#include <vector>
#include <Windows.h>

#include <map>
#include "valve_sdk/Misc/Color.hpp"
#include "features/skinchanger/item_definitions.hpp"
#include "imgui/imgui.h"

#define OPTION(type, var, val) type var = val

enum WEAPON_GROUPS {
	PISTOLS,
	RIFLES,
	SHOTGUNS,
	SCOUT,
	AUTO,
	AWP,
	SMG,
	UNKNOWN
};

class CVariables
{
public:
	struct Colour4
	{
		ImColor x, y, h, w;
		Colour4() { x = y = h = w = 0.0f; }
		Colour4(ImColor _x, ImColor _y, ImColor _h, ImColor _w) { x = _x; y = _y; h = _h; w = _w; }
	};
	Colour4 barColor1;
	Colour4 barColor2;

	int ragebot_antiaim_pitch = 0;
	int ragebot_antiaim_yaw = 0;
	bool ragebot_antiaim_desync = false;
	int ragebot_fakelag_amt = false;
	int ragebot_slowwalk_amt = 0;
	int ragebot_slowwalk_key = -1;

	bool ragebot_enabled = false;
	bool ragebot_backtrack = false;
	int ragebot_fov = 0;
	bool ragebot_autozeus = false;

	bool ragebot_hitbox[8][9] = { false, false, false, false, false, false, false, false };
	float ragebot_hitbox_multipoint_scale[8][9] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	int ragebot_selection[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	bool ragebot_autoscope[9] = { false, false, false, false, false, false, false, false };
	bool ragebot_autostop[9] = { false, false, false, false, false, false, false, false };
	bool ragebot_autocrouch[9] = { false, false, false, false, false, false, false, false };
	float ragebot_mindamage[9] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float ragebot_hitchance[9] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	int ragebot_baim_after_shots[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	float test = 200.f;
	float test2 = 200.f;
	float test3 = 0.f;
	int days = 0;

	//bool trigger_enabled = false;
	bool trigger_onkey = false;
	//bool trigger_flashcheck = false;
	//bool trigger_jump = false;
	//bool trigger_ignoreteam = false;
	//bool trigger_smokecheck = false;

	bool legit_enable = false;
	bool legit_smokecheck = false;
	bool legit_flashcheck = false;
	bool legit_ignoreteam = false;
	bool legit_drawfovaimbot = false;
	int legit_trigger_key = -1;
	bool trigger_keypressed = false;

	struct weapon_settings {
		bool aim_enabled = false;
		bool aim_autopistol = false;
		bool aim_ignore_walls = false;
		bool aim_perfect_silent = false;
		bool aim_recoil_based_fov = false;
		bool aim_rcs = false;
		bool aim_only_in_zoom = true;
		bool aim_nearest = true;
		bool fov_type = 0;
		int aim_rcs_type = 1;
		float aim_hitchance = 0;
		int aim_hitbox = 1;
		float aim_fov = 0.f;
		float aim_psilent_fov = 0.f;
		float aim_fov_2 = 0.f;
		float aim_smooth = 1;
		float aim_smooth_2 = 1;
		int aim_shot_delay = 0;
		int aim_kill_delay = 0;
		int aim_rcs_x = 100;
		int aim_rcs_y = 100;

		bool trigger_enable;
		bool trigger_Head;
		bool trigger_Body;
		bool trigger_Misc;
		int trigger_delay;
		float trigger_hitchance;

		char name[32] = "Default";
		bool enabled = false;
		int definition_vector_index = 0;
		int definition_index = 1;
		int paint_kit_vector_index = 0;
		int paint_kit_index = 0;
		int definition_override_vector_index = 0;
		int definition_override_index = 0;
		int seed = 0;
		int stat_trak = 0;
		float wear = 0;
		char custom_name[32] = "";
	};

	std::map<int, weapon_settings> weapon = {};

	struct {
		std::map<int, weapon_settings> m_items;
		std::unordered_map<std::string, std::string> m_icon_overrides;
		auto get_icon_override(const std::string original) const -> const char*
		{
			return m_icon_overrides.count(original) ? m_icon_overrides.at(original).data() : nullptr;
		}
	} skins;

	bool visuals_spreadxair = false;
	bool visuals_watermark = true;
	bool visuals_crosshair = true;
	bool visuals_norecoil = true;
	bool visuals_nightmode = false;
	bool visuals_noflash = false;

	bool misc_overridefov_inscope = false;
	bool visuals_thirdperson = false;
	int visuals_thirdperson_key = -1;

	bool visuals_noscopeborder = false;

	bool esp_enabled = false;
	bool esp_ignore_team = false;
	bool esp_visible_only = false;
	bool esp_player_boxes = false;
	bool esp_player_names = false;
	bool esp_player_health = false;
	bool esp_player_armour = false;
	bool esp_player_weapons = false;
	bool esp_player_snaplines = false;

	bool esp_drop_enable = false;
	int esp_drop_distance = 3500;

	bool esp_dropped_weapons = false;
	bool esp_defuse_kit = false;
	bool esp_planted_c4 = false;

	bool esp_case_pistol = false;
	bool esp_case_light_weapon = false;
	bool esp_case_heavy_weapon = false;
	bool esp_case_explosive = false;
	bool esp_case_tools = false;
	bool esp_random = false;
	bool esp_dz_armor_helmet = false;
	bool esp_dz_helmet = false;
	bool esp_dz_armor = false;
	bool esp_upgrade_tablet = false;
	bool esp_briefcase = false;
	bool esp_parachutepack = false;
	bool esp_dufflebag = false;
	bool esp_ammobox = false;

	bool glow_enabled = false;
	bool glow_ignore_team = false;
	bool glow_visible_only = false;
	bool chams_player_enabled = false;
	bool chams_player_ignore_team = false;
	bool chams_player_visible_only = false;

	bool misc_latency_enable = false;
	int misc_latency_amt = 0;
	bool legit_backtracking = false;
	bool misc_antikick = false;
	bool misc_autoaccept = false;
	bool misc_spectlist = false;
	bool misc_clantag = false;
	bool misc_fastduck = false;
	bool misc_bhop = false;
	bool misc_autostrafe = false;
	int misc_overridefov = 0;
	int misc_viewmodelfov = 0;
	bool misc_thirdperson = false;
	bool misc_showranks = true;
	float misc_thirdperson_dist = 50.f;
	float mat_ambient_light_r = 0.0f;
	float mat_ambient_light_g = 0.0f;
	float mat_ambient_light_b = 0.0f;

	bool radio_paused = true;
	int radio_selected = 0;

	float color_esp_enemy_visible[4] = { 1.f, 0.f, 0.f, 1.f };
	float color_esp_enemy_hidden[4] = { 0.f, 0.f, 0.f, 1.f };
	float color_esp_team_visible[4] = { 1.f, 0.f, 0.f, 1.f };
	float color_esp_team_hidden [4] = { 0.f, 0.f, 0.f, 1.f };

	float color_chams_enemy_visible[4]  = { 1.f, 0.f, 0.f, 1.f };
	float color_chams_enemy_hidden[4] = { 0.f, 0.f, 0.f, 1.f };
	float color_chams_team_visible[4]  = { 1.f, 0.f, 0.f, 1.f };
	float color_chams_team_hidden [4] = { 0.f, 0.f, 0.f, 1.f };

	float color_glow_enemy_visible[4]  = { 1.f, 0.f, 0.f, 1.f };
	float color_glow_enemy_hidden[4]  = { 1.f, 1.f, 1.f, 1.f };
	float color_glow_team_visible[4]  = { 1.f, 0.f, 0.f, 1.f };
	float color_glow_team_hidden[4]  = { 1.f, 1.f, 1.f, 1.f };
};

extern CVariables Vars;
extern CVariables::weapon_settings* WeaponSettings;
extern std::string ActiveWeaponName;

namespace Globals
{
	extern QAngle LastAngle;
	extern bool ThirdPersponToggle;
	extern bool Unload;
	extern bool MenuOpened;
	extern bool PlayerListOpened;
	extern bool RadioOpened;
	extern bool WeaponTabValid;
}
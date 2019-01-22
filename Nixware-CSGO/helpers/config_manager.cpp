#include "config_manager.hpp"
#include "../options.hpp"
#include "../imgui/imgui.h"

void CConfig::SetupValue(int &value, int def, std::string category, std::string name)
{
	value = def;
	ints.push_back(new ConfigValue<int>(category, name, &value));
}

void CConfig::SetupValue(bool &value, bool def, std::string category, std::string name)
{
	value = def;
	bools.push_back(new ConfigValue<bool>(category, name, &value));
}

void CConfig::SetupValue(float &value, float def, std::string category, std::string name)
{
	value = def;
	floats.push_back(new ConfigValue<float>(category, name, &value));
}

void CConfig::SetupColor(float value[4], std::string name) {
	for (int i = 0; i < 4; i++)
		SetupValue(value[i], value[i], "Colors", name + "_" + std::to_string(i).c_str());
}

void CConfig::SetupLegit()
{
	SetupValue(Vars.legit_enable, false, "LEGITBOT", "enabled");
	SetupValue(Vars.legit_ignoreteam, false, "LEGITBOT", "ignore_team");
	SetupValue(Vars.legit_smokecheck, false, "LEGITBOT", "smoke_check");
	SetupValue(Vars.legit_flashcheck, false, "LEGITBOT", "flash_check");
	SetupValue(Vars.legit_drawfovaimbot, false, "LEGITBOT", "draw_fov");
	SetupValue(Vars.legit_trigger_key, -1, "LEGITBOT", "trigger_key");

	for (DWORD i = 0; i < (sizeof(Utils::pWeaponData) / sizeof(*Utils::pWeaponData)); i++)
	{
		SetupValue(Vars.weapon[i].aim_enabled, false, Utils::pWeaponData[i], "aimbot_enable");
		SetupValue(Vars.weapon[i].aim_autopistol, false, Utils::pWeaponData[i], "aimbot_autopistol");
		SetupValue(Vars.weapon[i].aim_nearest, false, Utils::pWeaponData[i], "aimbot_nearest");
		SetupValue(Vars.weapon[i].aim_perfect_silent, false, Utils::pWeaponData[i], "aimbot_perfect_silent");

		SetupValue(Vars.weapon[i].aim_hitbox, 0, Utils::pWeaponData[i], "aimbot_hitbox");
		SetupValue(Vars.weapon[i].aim_ignore_walls, false, Utils::pWeaponData[i], "aimbot_ignorewalls");
		SetupValue(Vars.weapon[i].aim_recoil_based_fov, false, Utils::pWeaponData[i], "aimbot_recoil_based_fov");

		SetupValue(Vars.weapon[i].aim_hitchance, 0.f, Utils::pWeaponData[i], "aimbot_hitchance");

		SetupValue(Vars.weapon[i].aim_psilent_fov, 0.f, Utils::pWeaponData[i], "aimbot_psilent_fov");

		SetupValue(Vars.weapon[i].aim_fov, 0.f, Utils::pWeaponData[i], "aimbot_fov");
		SetupValue(Vars.weapon[i].aim_smooth, 0.f, Utils::pWeaponData[i], "aimbot_smooth");

		SetupValue(Vars.weapon[i].aim_fov_2, 0.f, Utils::pWeaponData[i], "aimbot_fov_2");
		SetupValue(Vars.weapon[i].aim_smooth_2, 0.f, Utils::pWeaponData[i], "aimbot_smooth_2");

		SetupValue(Vars.weapon[i].aim_rcs, false, Utils::pWeaponData[i], "aimbot_rcs");
		SetupValue(Vars.weapon[i].aim_rcs_x, 0.f, Utils::pWeaponData[i], "aimbot_rcs_pitch");
		SetupValue(Vars.weapon[i].aim_rcs_y, 0.f, Utils::pWeaponData[i], "aimbot_rcs_yaw");

		SetupValue(Vars.weapon[i].aim_shot_delay, 0.f, Utils::pWeaponData[i], "aimbot_shot_delay");
		SetupValue(Vars.weapon[i].aim_kill_delay, 0.f, Utils::pWeaponData[i], "aimbot_kill_delay");

		SetupValue(Vars.weapon[i].trigger_enable, false, Utils::pWeaponData[i], "trigger_enable");
		SetupValue(Vars.weapon[i].trigger_Head, false, Utils::pWeaponData[i], "trigger_head");
		SetupValue(Vars.weapon[i].trigger_Body, false, Utils::pWeaponData[i], "trigger_body");
		SetupValue(Vars.weapon[i].trigger_Misc, false, Utils::pWeaponData[i], "trigger_Misc");
		SetupValue(Vars.weapon[i].trigger_delay, 0, Utils::pWeaponData[i], "trigger_delay");
		SetupValue(Vars.weapon[i].trigger_hitchance, 0.f, Utils::pWeaponData[i], "trigger_hitchance");

		SetupValue(Vars.weapon[i].enabled, false, Utils::pWeaponData[i], "skin_enabled");
		SetupValue(Vars.weapon[i].seed, 0, Utils::pWeaponData[i], "skin_seed");
		SetupValue(Vars.weapon[i].stat_trak, 0, Utils::pWeaponData[i], "skin_stat_trak");
		SetupValue(Vars.weapon[i].wear, 0.001f, Utils::pWeaponData[i], "skin_wear");
		SetupValue(Vars.weapon[i].paint_kit_vector_index, 0, Utils::pWeaponData[i], "skin_index_selected");
	}
}

void CConfig::SetupRage()
{
	SetupValue(Vars.ragebot_enabled, false, "RAGEBOT", "enabled");
	SetupValue(Vars.ragebot_fov, 0, "RAGEBOT", "fov");
	SetupValue(Vars.ragebot_autozeus, false, "RAGEBOT", "autozeus");
	for (int type = WEAPON_GROUPS::PISTOLS; type <= WEAPON_GROUPS::SMG; type++) {
		SetupValue(Vars.ragebot_autoscope[type], false, "RAGEBOT", "autoscope_" + std::to_string(type));
		SetupValue(Vars.ragebot_autostop[type], false, "RAGEBOT", "autostop" + std::to_string(type));
		SetupValue(Vars.ragebot_autocrouch[type], false, "RAGEBOT", "autostop" + std::to_string(type));
		SetupValue(Vars.ragebot_hitchance[type], 0.f, "RAGEBOT", "hitchance" + std::to_string(type));
		SetupValue(Vars.ragebot_mindamage[type], 0.f, "RAGEBOT", "mindamage" + std::to_string(type));
		SetupValue(Vars.ragebot_baim_after_shots[type], 0, "RAGEBOT", "baim_after" + std::to_string(type));
		SetupValue(Vars.ragebot_selection[type], 0, "RAGEBOT", "targerselecion" + std::to_string(type));
		for (int i = 0; i < 8; i++) {
			SetupValue(Vars.ragebot_hitbox[i][type], false, "RAGEBOT", "hitscan_" + std::to_string(i) + std::to_string(type));
			SetupValue(Vars.ragebot_hitbox_multipoint_scale[i][type], 0.f, "RAGEBOT", "pointscale_" + std::to_string(i) + std::to_string(type));
		}
	}
	SetupValue(Vars.ragebot_fakelag_amt, 0, "RAGEBOT", "fakelag");
	SetupValue(Vars.ragebot_slowwalk_amt, 0, "RAGEBOT", "slowwalk");
	SetupValue(Vars.ragebot_slowwalk_key, 0, "RAGEBOT", "slowwalk_key");
	SetupValue(Vars.ragebot_antiaim_pitch, 0, "RAGEBOT", "antiaim_pitch");
	SetupValue(Vars.ragebot_antiaim_yaw, 0, "RAGEBOT", "antiaim_yaw");
	SetupValue(Vars.ragebot_antiaim_desync, false, "RAGEBOT", "desync");
}

void CConfig::SetupSkinchanger()
{
	SetupValue(Vars.weapon[WEAPON_KNIFE].enabled, false, "Knife CT", "skin_enabled");
	SetupValue(Vars.weapon[WEAPON_KNIFE].seed, 0, "Knife CT", "skin_seed");
	SetupValue(Vars.weapon[WEAPON_KNIFE].stat_trak, 0, "Knife CT", "skin_stat_trak");
	SetupValue(Vars.weapon[WEAPON_KNIFE].wear, 0.001f, "Knife CT", "skin_wear");
	SetupValue(Vars.weapon[WEAPON_KNIFE].paint_kit_vector_index, 0, "Knife CT", "skin_paintkit_selected_index");
	SetupValue(Vars.weapon[WEAPON_KNIFE].definition_override_vector_index, 0, "Knife CT", "skin_model_selected_index");

	SetupValue(Vars.weapon[WEAPON_KNIFE_T].enabled, false, "Knife TT", "skin_enabled");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].seed, 0, "Knife TT", "skin_seed");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].stat_trak, 0, "Knife TT", "skin_stat_trak");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].wear, 0.001f, "Knife TT", "skin_wear");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].paint_kit_vector_index, 0, "Knife TT", "skin_paintkit_selected_index");
	SetupValue(Vars.weapon[WEAPON_KNIFE_T].definition_override_vector_index, 0, "Knife TT", "skin_model_selected_index");

	SetupValue(Vars.weapon[5028].enabled, false, "Glove", "skin_enabled");
	SetupValue(Vars.weapon[5028].wear, 0.001f, "Glove", "skin_wear");
	SetupValue(Vars.weapon[5028].definition_override_vector_index, 0, "Glove", "skin_model_selected_index");
	SetupValue(Vars.weapon[5028].paint_kit_vector_index, 0, "Glove", "skin_paintkit_selected_index");

}

void CConfig::SetupMisc()
{
	SetupValue(Vars.misc_bhop, false, "MISC", "bunnyhop");
	SetupValue(Vars.misc_autostrafe, false, "MISC", "autostrafe");
	SetupValue(Vars.misc_clantag, false, "MISC", "clantag");
	SetupValue(Vars.misc_autoaccept, false, "MISC", "autoaccept");
	SetupValue(Vars.misc_antikick, false, "MISC", "antikick");

	SetupValue(Vars.misc_spectlist, false, "MISC", "spectatorlist");
	SetupValue(Vars.legit_backtracking, false, "MISC", "backtracking");
	SetupValue(Vars.misc_overridefov, 0, "MISC", "overridefov");
	SetupValue(Vars.misc_viewmodelfov, 0, "MISC", "viewmodelfov");
}

void CConfig::SetupVisuals()
{
	SetupValue(Vars.visuals_spreadxair, false, "VISUALS", "spreadxair");
	SetupValue(Vars.visuals_crosshair, false, "VISUALS", "crosshair");
	SetupValue(Vars.visuals_norecoil, false, "VISUALS", "norecoil");

	SetupValue(Vars.visuals_noscopeborder, false, "VISUALS", "noscopeborders");
	SetupValue(Vars.visuals_thirdperson, false, "VISUALS", "thirdperson");
	SetupValue(Vars.visuals_thirdperson_key, false, "VISUALS", "thirdperson_key");

	SetupValue(Vars.misc_overridefov_inscope, false, "VISUALS", "velocity");
	SetupValue(Vars.visuals_watermark, true, "VISUALS", "watermark");

	SetupValue(Vars.esp_enabled, false, "ESP", "enabled");
	SetupValue(Vars.esp_ignore_team, false, "ESP", "ignore_team");
	SetupValue(Vars.esp_visible_only, false, "ESP", "visible_only");
	SetupValue(Vars.esp_player_boxes, false, "ESP", "boxes");
	SetupValue(Vars.esp_player_names, false, "ESP", "names");
	SetupValue(Vars.esp_player_weapons, false, "ESP", "weapon");
	SetupValue(Vars.esp_player_health, false, "ESP", "health");

	SetupValue(Vars.chams_player_enabled, false, "CHAMS", "enabled");
	SetupValue(Vars.chams_player_ignore_team, false, "CHAMS", "ignore_team");
	SetupValue(Vars.chams_player_visible_only, false, "CHAMS", "visible_only");

	SetupValue(Vars.glow_enabled, false, "GLOW", "enabled");
	SetupValue(Vars.glow_ignore_team, false, "GLOW", "ignore_team");
	SetupValue(Vars.glow_visible_only, false, "GLOW", "visible_only");

	SetupValue(Vars.esp_drop_enable, false, "Item ESP", "drop_enable");
	SetupValue(Vars.esp_drop_distance, 0, "Item ESP", "drop_distance");

	SetupValue(Vars.esp_dropped_weapons, false, "Item ESP", "dropped_weapons");
	SetupValue(Vars.esp_planted_c4, false, "Item ESP", "planted_c4");
	SetupValue(Vars.esp_case_pistol, false, "Item ESP", "case_pistol");
	SetupValue(Vars.esp_case_light_weapon, false, "Item ESP", "light_weapon");
	SetupValue(Vars.esp_case_heavy_weapon, false, "Item ESP", "heavy_weapon");
	SetupValue(Vars.esp_case_explosive, false, "Item ESP", "case_explosive");
	SetupValue(Vars.esp_case_tools, false, "Item ESP", "case_tools");
	SetupValue(Vars.esp_random, false, "Item ESP", "random");
	SetupValue(Vars.esp_dz_armor_helmet, false, "Item ESP", "dz_armor_helmet");
	SetupValue(Vars.esp_dz_helmet, false, "Item ESP", "dz_helmet");
	SetupValue(Vars.esp_dz_armor, false, "Item ESP", "dz_armor");
	SetupValue(Vars.esp_upgrade_tablet, false, "Item ESP", "upgrade_tablet");
	SetupValue(Vars.esp_briefcase, false, "Item ESP", "briefcase");
	SetupValue(Vars.esp_parachutepack, false, "Item ESP", "parachutepack");
	SetupValue(Vars.esp_dufflebag, false, "Item ESP", "dufflebag");
	SetupValue(Vars.esp_ammobox, false, "Item ESP", "ammobox");
}

void CConfig::SetupColors()
{
	SetupColor(Vars.color_esp_enemy_visible, "color_esp_enemy_visible");
	SetupColor(Vars.color_esp_enemy_hidden, "color_esp_enemy_hidden");
	SetupColor(Vars.color_esp_team_visible, "color_esp_team_visible");
	SetupColor(Vars.color_esp_team_hidden, "color_esp_team_hidden");

	SetupColor(Vars.color_chams_enemy_visible, "color_chams_enemy_visible");
	SetupColor(Vars.color_chams_enemy_hidden, "color_chams_enemy_hidden");
	SetupColor(Vars.color_chams_team_visible, "color_chams_team_visible");
	SetupColor(Vars.color_chams_team_hidden, "color_chams_team_hidden");

	SetupColor(Vars.color_glow_enemy_visible, "color_glow_enemy_visible");
	SetupColor(Vars.color_glow_enemy_hidden, "color_glow_enemy_hidden");
	SetupColor(Vars.color_glow_team_visible, "color_glow_team_visible");
	SetupColor(Vars.color_glow_team_hidden, "color_glow_team_hidden");
}

void CConfig::Initialize()
{
	CConfig::SetupLegit();
	CConfig::SetupRage();
	CConfig::SetupSkinchanger();
	CConfig::SetupMisc();
	CConfig::SetupVisuals();
	CConfig::SetupColors();
}

void CConfig::Save(std::string szIniFile)
{
	std::string folder, file;
	folder = XorStr("C:/Nixware/");

	file = folder + szIniFile + XorStr(".nixware");

	CreateDirectoryA(folder.c_str(), NULL);

	for (auto value : ints)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : floats)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : bools)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "1" : "0", file.c_str());
}

void CConfig::Load(std::string szIniFile)
{
	std::string folder, file;

	folder = XorStr("C:/Nixware/");

	file = folder + szIniFile + XorStr(".nixware");

	CreateDirectoryA(folder.c_str(), NULL);

	char value_l[32] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atoi(value_l);
	}

	for (auto value : floats)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = (float)atof(value_l);
	}

	for (auto value : bools)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "1");
	}
}
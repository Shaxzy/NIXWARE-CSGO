#pragma once
#include <map>
#include <vector>
#include "../../valve_sdk/csgostructs.hpp"

static auto is_knife(const int i) -> bool
{
	return (i >= WEAPON_KNIFE_BAYONET && i < 5027) || i == WEAPON_KNIFE_T || i == WEAPON_KNIFE;
}
struct weapon_info
{
	constexpr weapon_info(const char* model, const char* icon = nullptr) :
		model(model),
		icon(icon)
	{}
	const char* model;
	const char* icon;
};
struct weapon_name
{
	constexpr weapon_name(const int definition_index, const char* name) :
		definition_index(definition_index),
		name(name)
	{}
	int definition_index = 0;
	const char* name = nullptr;
};
extern const std::map<size_t, weapon_info> k_weapon_info;
extern const std::vector<weapon_name> k_knife_names;
extern const std::vector<weapon_name> k_glove_names;
extern const std::vector<weapon_name> k_weapon_names_s;

#pragma once
#include "../../valve_sdk/csgostructs.hpp"
#include "../../options.hpp"

struct paint_kit
{
	int id;
	std::string name;
	auto operator < (const paint_kit& other) const -> bool
	{
		return name < other.name;
	}
};
extern std::vector<paint_kit> k_skins;
extern std::vector<paint_kit> k_gloves;

class CSkinChanger : public Singleton<CSkinChanger>
{
public:
	void HookSequence();
	void Update();
	void OnFrameStageNotify(ClientFrameStage_t stage);
	void Initialize();
private:
	int GetWeaponSettingsSelectID_ForSkins(C_BaseAttributableItem * weapon);
};

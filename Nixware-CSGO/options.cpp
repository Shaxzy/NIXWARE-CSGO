#include "options.hpp"
#include "helpers/utils.hpp"

CVariables Vars;
CVariables::weapon_settings* WeaponSettings = &Vars.weapon[WEAPON_NONE];
std::string ActiveWeaponName = "Weapon";

namespace Globals
{
	QAngle LastAngle = QAngle(0, 0, 0);
	bool ThirdPersponToggle = true;
	bool Unload = false;
	bool MenuOpened = false;
	bool PlayerListOpened = false;
	bool RadioOpened = false;
	bool WeaponTabValid = false;
}

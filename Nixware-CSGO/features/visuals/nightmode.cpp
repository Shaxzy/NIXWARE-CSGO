#include "nightmode.hpp"
#include "../../valve_sdk/csgostructs.hpp"

void CNightmode::UpdateWorldTextures()
{
	if (Vars.visuals_nightmode) {
		if (!NightmodeDone) {
			PerformNightmode();
			NightmodeDone = true;
		}
	}
}

void CNightmode::PerformNightmode()
{
	static auto r_DrawSpecificStaticProp = Interfaces::Convar->FindVar(XorStr("r_DrawSpecificStaticProp"));
	r_DrawSpecificStaticProp->SetValue(Vars.visuals_nightmode);
	auto sv_skyname = Interfaces::Convar->FindVar(XorStr("sv_skyname"));
	sv_skyname->SetValue(Vars.visuals_nightmode ? XorStr("sky_csgo_night02") : OldSkyname.c_str());

	for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
	{
		IMaterial *pMaterial = Interfaces::MaterialSystem->GetMaterial(i);

		if (!pMaterial)
			continue;

		const char* group = pMaterial->GetTextureGroupName();
		const char* name = pMaterial->GetName();
		float world_textures = Vars.visuals_nightmode ? 0.10f : 1.f;
		float staticprop = Vars.visuals_nightmode ? 0.30f : 1.f;
		float palace_pillars = Vars.visuals_nightmode ? 0.30f : 1.f;
		if (strstr(group, XorStr("World textures")))
		{
			pMaterial->ColorModulate(world_textures, world_textures, world_textures);
		}
		if (strstr(group, XorStr("StaticProp")))
		{
			pMaterial->ColorModulate(staticprop, staticprop, staticprop);
		}
		if (strstr(name, XorStr("models/props/de_dust/palace_bigdome")))
		{
			pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, Vars.visuals_nightmode);
		}
		if (strstr(name, XorStr("models/props/de_dust/palace_pillars")))
		{
			pMaterial->ColorModulate(palace_pillars, palace_pillars, palace_pillars);
		}

		if (strstr(group, XorStr("Particle textures")))
		{
			pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, Vars.visuals_nightmode);
		}
	}
}
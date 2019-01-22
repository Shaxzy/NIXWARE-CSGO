#include "chams.hpp"
#include "../../options.hpp"

float color_vis[4] = { 1.f, 0.f, 0.f, 1.f };
float color_invis[4] = { 0.f, 0.f, 0.f, 1.f };

void CChams::SceneEnd()
{
	if (!Vars.chams_player_enabled)
		return;

	for (int i = 1; i < Interfaces::GlobalVars->maxClients; ++i)
	{
		auto ent = static_cast<C_BasePlayer*>(Interfaces::EntityList->GetClientEntity(i));
		if (ent && ent->IsAlive() && !(ent->IsTeammate() && Vars.chams_player_ignore_team) && !ent->IsDormant()/*&& SmokeCheck(ent)*/)
		{
			if (ent->IsTeammate()) {
				color_vis[0] = Vars.color_chams_team_visible[0];
				color_vis[1] = Vars.color_chams_team_visible[1];
				color_vis[2] = Vars.color_chams_team_visible[2];
				color_vis[3] = Vars.color_chams_team_visible[3];
			}
			else {
				color_vis[0] = Vars.color_chams_enemy_visible[0];
				color_vis[1] = Vars.color_chams_enemy_visible[1];
				color_vis[2] = Vars.color_chams_enemy_visible[2];
				color_vis[3] = Vars.color_chams_enemy_visible[3];
			}

			if (ent->IsTeammate()) {
				color_invis[0] = Vars.color_chams_team_hidden[0];
				color_invis[1] = Vars.color_chams_team_hidden[1];
				color_invis[2] = Vars.color_chams_team_hidden[2];
				color_invis[3] = Vars.color_chams_team_hidden[3];
			}
			else {
				color_invis[0] = Vars.color_chams_enemy_hidden[0];
				color_invis[1] = Vars.color_chams_enemy_hidden[1];
				color_invis[2] = Vars.color_chams_enemy_hidden[2];
				color_invis[3] = Vars.color_chams_enemy_hidden[3];
			}


			if (material_norm) {

				if (Vars.chams_player_visible_only)
				{
					Interfaces::RenderView->SetColorModulation(color_vis);
					Interfaces::ModelRender->ForcedMaterialOverride(material_norm);
					ent->DrawModel(0x1, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				}
				else
				{
					Interfaces::RenderView->SetColorModulation(color_invis);
					material_norm->IncrementReferenceCount();
					material_norm->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					Interfaces::ModelRender->ForcedMaterialOverride(material_norm);
					ent->DrawModel(0x1, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					Interfaces::RenderView->SetColorModulation(color_vis);
					material_norm->IncrementReferenceCount();
					material_norm->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
					Interfaces::ModelRender->ForcedMaterialOverride(material_norm);
					ent->DrawModel(0x1, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				}
			}
		}
	}
}

void CChams::Initialize()
{
	material_norm = Interfaces::MaterialSystem->FindMaterial(XorStr("debug/debugambientcube"), TEXTURE_GROUP_MODEL);
}

void CChams::Shutdown()
{
	CChams::Get().material_norm = nullptr;
	delete CChams::Get().material_norm;
}

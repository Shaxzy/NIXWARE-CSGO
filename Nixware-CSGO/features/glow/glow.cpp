#include "glow.hpp"

#include "../../valve_sdk/csgostructs.hpp"
#include "../../options.hpp"

void CGlow::Shutdown()
{
    // Remove glow from all entities
    for(auto i = 0; i < Interfaces::GlowManager->m_GlowObjectDefinitions.Count(); i++) {
        auto& glowObject = Interfaces::GlowManager->m_GlowObjectDefinitions[i];
        auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

        if(glowObject.IsUnused())
            continue;

        if(!entity || entity->IsDormant())
            continue;

        glowObject.m_flAlpha = 0.0f;
    }
}

void CGlow::Run()
{
	if (!Vars.glow_enabled)
		return;

	if (!Utils::IsInGame())
		return;

    for(auto i = 0; i < Interfaces::GlowManager->m_GlowObjectDefinitions.Count(); i++) {
        auto& glowObject = Interfaces::GlowManager->m_GlowObjectDefinitions[i];
        auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

        if(glowObject.IsUnused())
            continue;

        if(!entity || entity->IsDormant())
            continue;

        auto class_id = entity->GetClientClass()->m_ClassID;
        auto color = Color{};

		if (class_id == CCSPlayer)
		{
			if (!entity->IsAlive())
				continue;

			if (Vars.glow_ignore_team && entity->IsTeammate())
				continue;

			if (Vars.glow_visible_only && !entity->IsVisible())
				continue;

			Color team_color = entity->IsVisible()
				? Color(Vars.color_glow_team_visible[0],
					Vars.color_glow_team_visible[1],
					Vars.color_glow_team_visible[2],
					Vars.color_glow_team_visible[3])
				:
				Color(Vars.color_glow_team_hidden[0],
					Vars.color_glow_team_hidden[1],
					Vars.color_glow_team_hidden[2],
					Vars.color_glow_team_hidden[3]);


			Color enemy_color = entity->IsVisible()
				? Color(Vars.color_glow_enemy_visible[0],
					Vars.color_glow_enemy_visible[1],
					Vars.color_glow_enemy_visible[2],
					Vars.color_glow_enemy_visible[3])
				:
				Color(Vars.color_glow_enemy_hidden[0],
					Vars.color_glow_enemy_hidden[1],
					Vars.color_glow_enemy_hidden[2],
					Vars.color_glow_enemy_hidden[3]);

			color = entity->IsTeammate() ? team_color : enemy_color;
		}

        glowObject.m_flRed = color.r() / 255.0f;
        glowObject.m_flGreen = color.g() / 255.0f;
        glowObject.m_flBlue = color.b() / 255.0f;
        glowObject.m_flAlpha = color.a() / 255.0f;
        glowObject.m_bRenderWhenOccluded = true;
        glowObject.m_bRenderWhenUnoccluded = false;
    }
}

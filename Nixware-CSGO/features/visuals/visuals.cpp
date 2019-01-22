#include <Windows.h>
#include <ctime>

#include "visuals.hpp"
#include "../legitbot/legitbot.hpp"

#include "../../options.hpp"
#include "../../helpers/math.hpp"
#include "../../helpers/utils.hpp"

#include "../../menu/menu.hpp"

RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top > screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom < screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

C_BasePlayer* local_observed;
float ESP_Fade[64];

int GetDistance(C_BaseEntity* from, C_BaseEntity* to)
{
	return from->GetRenderOrigin().DistTo(to->GetRenderOrigin());
}

struct
{
	C_BasePlayer* pl;
	bool          is_enemy;
	bool          is_team;
	bool          is_visible;
	Color         esp_color;
	Color         clr2;
	Color         textClr;
	Color         flashClr;
	Color         healthbar_color;
	Color         hpClr2;
	Color         armClr;
	Color         armClr2;
	Color         skeletClr;
	Vector        head_pos;
	Vector        feet_pos;
	RECT          bbox;
} esp_ctx;

enum PLAYER_TEAM
{
	TEAM_SPEC = 1,
	TEAM_TT,
	TEAM_CT
};

bool CVisuals::ValidPlayer(C_BasePlayer *player, bool count_step)
{
	int idx = player->EntIndex();
	constexpr float frequency = 0.35f / 0.5f;
	float step = frequency * Interfaces::GlobalVars->frametime;
	if (!player->IsAlive())
		return false;

	// Don't render esp if in firstperson viewing player.
	if (player == local_observed)
	{
		if (Globals::LocalPlayer->m_iObserverMode() == 4)
			return false;
	}

	if (player == Globals::LocalPlayer)
	{
		if (!Interfaces::Input->m_fCameraInThirdPerson)
			return false;
	}

	if (count_step)
	{
		if (!player->IsDormant()) {
			if (ESP_Fade[idx] < 1.f)
				ESP_Fade[idx] += step;
		}
		else {
			if (ESP_Fade[idx] > 0.f)
				ESP_Fade[idx] -= step;
		}
		ESP_Fade[idx] = (ESP_Fade[idx] > 1.f ? 1.f : ESP_Fade[idx] < 0.f ? 0.f : ESP_Fade[idx]);
	}

	return (ESP_Fade[idx] > 0.f);
}

void CVisuals::DrawSkeleton(C_BasePlayer* entity, Color color, const matrix3x4_t* bone_matrix)
{
	auto studio_model = Interfaces::ModelInfo->GetStudiomodel(entity->GetModel());
	if (!studio_model)
		return;

	for (int i = 0; i < studio_model->numbones; i++)
	{
		auto bone = studio_model->GetBone(i);
		if (!bone || bone->parent < 0 || !(bone->flags & BONE_USED_BY_HITBOX))
			continue;

		/// 2 bone matrices, 1 for the child and 1 for the parent bone
		matrix3x4_t bone_matrix_1, bone_matrix_2;
		if (bone_matrix)
		{
			bone_matrix_1 = bone_matrix[i];
			bone_matrix_2 = bone_matrix[bone->parent];
		}
		else
		{
			bone_matrix_1 = entity->GetBoneMatrix(i);
			bone_matrix_2 = entity->GetBoneMatrix(bone->parent);
		}

		Vector bone_position_1 = Vector(bone_matrix_1[0][3], bone_matrix_1[1][3], bone_matrix_1[2][3]),
			bone_position_2 = Vector(bone_matrix_2[0][3], bone_matrix_2[1][3], bone_matrix_2[2][3]);

		if (Vector screen1
			,
			screen2;
			Math::WorldToScreen(bone_position_1, screen1) && Math::WorldToScreen(bone_position_2, screen2)
			)
			CDraw::Get().Line(screen1.x, screen1.y, screen2.x, screen2.y, color);
	}
}

void CVisuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	wchar_t buf[80];
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Interfaces::Surface->DrawSetColor(Color::White);
	Interfaces::Surface->DrawOutlinedRect(bbox.left, bbox.top, bbox.right, bbox.bottom);
	Interfaces::Surface->DrawSetColor(Color::Black);
	Interfaces::Surface->DrawOutlinedRect(bbox.left - 1, bbox.top - 1, bbox.right + 1, bbox.bottom + 1);
	Interfaces::Surface->DrawOutlinedRect(bbox.left + 1, bbox.top + 1, bbox.right - 1, bbox.bottom - 1);

	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);

	if (MultiByteToWideChar(CP_UTF8, 0, name, -1, buf, 80) > 0) {
		int w = bbox.right - bbox.left;
		int tw, th;

		Interfaces::Surface->GetTextSize(CDraw::Get().font.ESPFLAG, buf, tw, th);
		Interfaces::Surface->DrawSetTextFont(CDraw::Get().font.ESPFLAG);
		Interfaces::Surface->DrawSetTextColor(Color::White);
		Interfaces::Surface->DrawSetTextPos((bbox.left + w * 0.5f) - tw * 0.5f, bbox.bottom + 1);
		Interfaces::Surface->DrawPrintText(buf, wcslen(buf));
	}
}

void CVisuals::RenderWeapon()
{
	wchar_t buf[80];
	auto weapon = esp_ctx.pl->m_hActiveWeapon();
	if (!weapon) return;
	if (MultiByteToWideChar(CP_UTF8, 0, weapon->GetCSWeaponData()->szWeaponName + 7, -1, buf, 80) > 0) {
		int tw, th;
		Interfaces::Surface->GetTextSize(CDraw::Get().font.ESPFLAG, buf, tw, th);
		Interfaces::Surface->DrawSetTextFont(CDraw::Get().font.ESPFLAG);
		Interfaces::Surface->DrawSetTextColor(esp_ctx.textClr);
		if (Vars.esp_player_health)
			Interfaces::Surface->DrawSetTextPos(esp_ctx.bbox.left + (esp_ctx.bbox.right - esp_ctx.bbox.left) / 2 - tw / 2, esp_ctx.bbox.bottom + 9);
		else
			Interfaces::Surface->DrawSetTextPos(esp_ctx.bbox.left + (esp_ctx.bbox.right - esp_ctx.bbox.left) / 2 - tw / 2, esp_ctx.bbox.bottom + 2);

		Interfaces::Surface->DrawPrintText(buf, wcslen(buf));
	}
}

void CVisuals::RenderDrop()
{
	if (!Vars.esp_drop_enable)
		return;

	if (!Utils::IsInGame())
		return;

	for (int EntIndex = 0; EntIndex < Interfaces::EntityList->GetHighestEntityIndex(); EntIndex++)
	{
		C_BaseEntity* entity = (C_BaseEntity*)Interfaces::EntityList->GetClientEntity(EntIndex);

		if (!entity || entity->IsPlayer())
			continue;

		if (GetDistance(Globals::LocalPlayer, entity) > Vars.esp_drop_distance)
			continue;

		if (Vars.esp_dropped_weapons && entity->IsWeapon())
			RenderWeapon((C_BaseCombatWeapon*)entity);
		else if (Vars.esp_planted_c4 && entity->IsPlantedC4()) 
			RenderPlantedC4(entity);
		
		const model_t* pModel = entity->GetModel();

		if (pModel)
		{
			const char* pModelName = Interfaces::ModelInfo->GetModelName(pModel);

			if (pModelName)
			{
				Vector vEntScreen;
				std::string WeaponName = "";
				if (Math::WorldToScreen(entity->GetRenderOrigin(), vEntScreen))
				{
					if (Vars.esp_case_pistol && !strcmp(pModelName, "models/props_survival/cases/case_pistol.mdl"))
						WeaponName = "Pistol Case";					
					else if (Vars.esp_case_light_weapon && !strcmp(pModelName, "models/props_survival/cases/case_light_weapon.mdl"))
						WeaponName = "Light Case";
					else if (Vars.esp_case_heavy_weapon && !strcmp(pModelName, "models/props_survival/cases/case_heavy_weapon.mdl"))
						WeaponName = "Heavy Case";
					else if (Vars.esp_case_explosive && !strcmp(pModelName, "models/props_survival/cases/case_explosive.mdl"))
						WeaponName = "Explosive Case";
					else if (Vars.esp_case_tools && !strcmp(pModelName, "models/props_survival/cases/case_tools.mdl"))
						WeaponName = "Tools Case";
					else if (Vars.esp_random && !strcmp(pModelName, "models/props_survival/cases/random.mdl"))
						WeaponName = "Airdrop";
					else if (Vars.esp_dz_armor_helmet && !strcmp(pModelName, "models/props_survival/cases/dz_armor_helmet.mdl"))
						WeaponName = "Full Armor";
					else if (Vars.esp_dz_helmet && !strcmp(pModelName, "models/props_survival/cases/dz_helmet.mdl"))
						WeaponName = "Helmet";
					else if (Vars.esp_dz_armor && !strcmp(pModelName, "models/props_survival/cases/dz_armor.mdl"))
						WeaponName = "Armor";
					else if (Vars.esp_upgrade_tablet && !strcmp(pModelName, "models/props_survival/cases/upgrade_tablet.mdl"))
						WeaponName = "Tablet Upgrade";
					else if (Vars.esp_briefcase && !strcmp(pModelName, "models/props_survival/cases/briefcase.mdl"))
						WeaponName = "Briefcase";
					else if (Vars.esp_parachutepack && !strcmp(pModelName, "models/props_survival/cases/parachutepack.mdl"))
						WeaponName = "Parachute";
					else if (Vars.esp_dufflebag && !strcmp(pModelName, "models/props_survival/cases/dufflebag.mdl"))
						WeaponName = "Cash Dufflebag";
					else if (Vars.esp_ammobox && entity->IsLoot() && !strcmp(pModelName, "models/props_survival/crates/crate_ammobox.mdl"))
						WeaponName = "Ammobox";

					auto bbox = GetBBox(entity);

					if (WeaponName != "") {
						Interfaces::Surface->DrawSetColor(Color::White);
						Interfaces::Surface->DrawOutlinedRect(bbox.left, bbox.top, bbox.right, bbox.bottom);
						Interfaces::Surface->DrawSetColor(Color::Black);
						Interfaces::Surface->DrawOutlinedRect(bbox.left - 1, bbox.top - 1, bbox.right + 1, bbox.bottom + 1);
						Interfaces::Surface->DrawOutlinedRect(bbox.left + 1, bbox.top + 1, bbox.right - 1, bbox.bottom - 1);
					}

					//CDraw::Get().DrawString(CDraw::Get().font.ESPFLAG, (int)vEntScreen.x, (int)vEntScreen.y, Color::White, 2, pModelName);

					wchar_t buf[128];

					if (MultiByteToWideChar(CP_UTF8, 0, WeaponName.c_str(), -1, buf, 80) > 0) {
						int w = bbox.right - bbox.left;
						int tw, th;

						Interfaces::Surface->GetTextSize(CDraw::Get().font.ESPFLAG, buf, tw, th);
						Interfaces::Surface->DrawSetTextFont(CDraw::Get().font.ESPFLAG);
						Interfaces::Surface->DrawSetTextColor(Color::White);
						Interfaces::Surface->DrawSetTextPos((bbox.left + w * 0.5f) - tw * 0.5f, bbox.bottom + 1);
						Interfaces::Surface->DrawPrintText(buf, wcslen(buf));
					}
				}
			}
		}
	}
}

void CVisuals::RenderDefuseKit(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	g_VGuiSurface->DrawSetColor(Color::Blue);
	g_VGuiSurface->DrawLine(bbox.left, bbox.bottom, bbox.left, bbox.top);
	g_VGuiSurface->DrawLine(bbox.left, bbox.top, bbox.right, bbox.top);
	g_VGuiSurface->DrawLine(bbox.right, bbox.top, bbox.right, bbox.bottom);
	g_VGuiSurface->DrawLine(bbox.right, bbox.bottom, bbox.left, bbox.bottom);

	const wchar_t* buf = L"Defuse Kit";

	int w = bbox.right - bbox.left;
	int tw, th;
	g_VGuiSurface->GetTextSize(CDraw::Get().font.ESPFLAG, buf, tw, th);

	g_VGuiSurface->DrawSetTextFont(CDraw::Get().font.ESPFLAG);
	g_VGuiSurface->DrawSetTextColor(esp_ctx.esp_color);
	g_VGuiSurface->DrawSetTextPos((bbox.left + w * 0.5f) - tw * 0.5f, bbox.bottom + 1);
	g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
}

void CVisuals::RenderPlantedC4(C_BaseEntity* ent)
{
	auto bbox = GetBBox(ent);

	Vector screenPositon;

	if (Math::WorldToScreen(ent->m_vecOrigin(), screenPositon))
	{
		const wchar_t* buf = L"Planted bomb";
		int tw, th;
		Interfaces::Surface->GetTextSize(CDraw::Get().font.ESPFLAG, buf, tw, th);

		Interfaces::Surface->DrawSetTextFont(CDraw::Get().font.ESPFLAG);
		Interfaces::Surface->DrawSetTextColor(Color::Blue);
		Interfaces::Surface->DrawSetTextPos(screenPositon.x - tw / 2, screenPositon.y - th / 2);
		Interfaces::Surface->DrawPrintText(buf, wcslen(buf));
	}
}

bool CVisuals::Begin(C_BasePlayer* pl)
{
	esp_ctx.pl = pl;
	esp_ctx.is_enemy = !pl->IsTeammate();
	esp_ctx.is_visible = pl->IsVisible();

	if (!esp_ctx.is_enemy && Vars.esp_ignore_team && !Utils::IsInDangerZone())
		return false;

	if (Vars.esp_visible_only && !pl->IsVisible())
		return false;

	Color team_color = pl->IsVisible()
		? Color(Vars.color_esp_team_visible[0],
			Vars.color_esp_team_visible[1],
			Vars.color_esp_team_visible[2],
			Vars.color_esp_team_visible[3])
		:
		Color(Vars.color_esp_team_hidden[0],
			Vars.color_esp_team_hidden[1],
			Vars.color_esp_team_hidden[2],
			Vars.color_esp_team_hidden[3]);


	Color enemy_color = pl->IsVisible() 
		? Color(Vars.color_esp_enemy_visible[0],
			Vars.color_esp_enemy_visible[1],
			Vars.color_esp_enemy_visible[2],
			Vars.color_esp_enemy_visible[3]) 
		: 
		Color(Vars.color_esp_enemy_hidden[0],
			Vars.color_esp_enemy_hidden[1], 
			Vars.color_esp_enemy_hidden[2],
			Vars.color_esp_enemy_hidden[3]);

	esp_ctx.esp_color = pl->IsTeammate() ? team_color : enemy_color;

	esp_ctx.textClr = Color::White;
	esp_ctx.clr2 = Color::Black;

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	head.z += 15;

	if (!Math::WorldToScreen(head, esp_ctx.head_pos) ||
		!Math::WorldToScreen(origin, esp_ctx.feet_pos))
		return false;

	auto h = fabs(esp_ctx.head_pos.y - esp_ctx.feet_pos.y);
	auto w = h / 1.65f;

	esp_ctx.bbox = GetBBox((C_BasePlayer*)pl);

	return true;
}

void CVisuals::RenderBox()
{
	Interfaces::Surface->DrawSetColor(esp_ctx.esp_color);
	Interfaces::Surface->DrawOutlinedRect(esp_ctx.bbox.left, esp_ctx.bbox.top, esp_ctx.bbox.right, esp_ctx.bbox.bottom);
	Interfaces::Surface->DrawSetColor(Color(0, 0, 0, 185));
	Interfaces::Surface->DrawOutlinedRect(esp_ctx.bbox.left - 1, esp_ctx.bbox.top - 1, esp_ctx.bbox.right + 1, esp_ctx.bbox.bottom + 1);
	Interfaces::Surface->DrawOutlinedRect(esp_ctx.bbox.left + 1, esp_ctx.bbox.top + 1, esp_ctx.bbox.right - 1, esp_ctx.bbox.bottom - 1);
}

void CVisuals::HealthBar()
{
	int height = (esp_ctx.bbox.bottom - esp_ctx.bbox.top);
	int CurHealth = esp_ctx.pl->m_iHealth();

	float Width = (float)fabs(esp_ctx.bbox.right - esp_ctx.bbox.left);

	if (CurHealth > 100)
		CurHealth = 100;
	int iHealthWidth = (CurHealth * (Width*0.5*0.1) * 0.2);
	int iHealthMaxWidth = (100 * (Width*0.5*0.1) * 0.2);

	Color clr = !esp_ctx.is_enemy
		? Color(Vars.color_esp_team_visible[0],
			Vars.color_esp_team_visible[1],
			Vars.color_esp_team_visible[2],
			Vars.color_esp_team_visible[3])
		:
		Color(Vars.color_esp_enemy_visible[0],
			Vars.color_esp_enemy_visible[1],
			Vars.color_esp_enemy_visible[2],
			Vars.color_esp_enemy_visible[3]);

	CDraw::Get().DrawHeader((esp_ctx.bbox.left), esp_ctx.bbox.top + height + 3, iHealthMaxWidth + 2, iHealthWidth, clr);
}

void CVisuals::RenderName()
{
	wchar_t buf[128];

	player_info_t info = esp_ctx.pl->GetPlayerInfo();

	if (MultiByteToWideChar(CP_UTF8, 0, info.szName, -1, buf, 128) > 0) {
		int tw, th;
		Interfaces::Surface->GetTextSize(CDraw::Get().font.ESPFLAG, buf, tw, th);

		Interfaces::Surface->DrawSetTextFont(CDraw::Get().font.ESPFLAG);
		Interfaces::Surface->DrawSetTextColor(esp_ctx.textClr);
		Interfaces::Surface->DrawSetTextPos(esp_ctx.bbox.left + (esp_ctx.bbox.right - esp_ctx.bbox.left) / 2 - tw / 2, esp_ctx.bbox.top - th);
		Interfaces::Surface->DrawPrintText(buf, wcslen(buf));
	}
}

void CVisuals::RenderSpreadXair()
{
	if (!Vars.visuals_spreadxair) return;

	if (Globals::LocalPlayer && Globals::LocalPlayer->IsAlive() /*&& Misc::ShowSpread == 1*/)
	{
		if (Globals::LocalPlayer->m_hActiveWeapon()->IsGun())
		{
			float cone = Globals::LocalPlayer->m_hActiveWeapon()->GetInaccuracy();
			if (cone > 0.0f)
			{
				Vector screen;
				if (cone < 0.01f) cone = 0.01f;

				float size1337 = (cone * Globals::ScreenHeight) * 0.7f;

				CDraw::Get().DrawFilledCircle(Vector2D(Globals::ScreenWeight / 2, Globals::ScreenHeight / 2), Color(24, 24, 24, 124), size1337, 60);
			}
		}
	}
}

void CVisuals::DrawAimbotFov()
{
	if (!Utils::IsInGame())	return;	
	if (!Globals::LocalPlayer)return;
	if (!Globals::LocalPlayer->IsAlive()) return;
	if (!Globals::LocalPlayer->m_hActiveWeapon()->IsGun()) return;

	if (Utils::IsInGame() && Vars.legit_enable && Vars.legit_drawfovaimbot && WeaponSettings->aim_enabled)
	{
		float w2, h2;

		if (WeaponSettings->aim_recoil_based_fov)
		{
			static QAngle ViewAngles;
			Interfaces::Engine->GetViewAngles(ViewAngles);
			ViewAngles += (Globals::LocalPlayer->m_aimPunchAngle()) * 2.f;

			static Vector fowardVec;
			Math::AngleVectors(ViewAngles, fowardVec);
			fowardVec *= 10000;


			// Get ray start / end
			Vector start = Globals::LocalPlayer->m_vecOrigin() + Globals::LocalPlayer->m_vecViewOffset();
			Vector end = start + fowardVec, endScreen;

			if (Math::WorldToScreen(end, endScreen)) {
				w2 = endScreen.x;
				h2 = endScreen.y;
			}
		}
		else {
			w2 = Globals::ScreenWeight / 2;
			h2 = Globals::ScreenHeight / 2;
		}

		float diagonal = sqrt(pow(Globals::ScreenWeight, 2) + pow(Globals::ScreenHeight, 2));
		float fov = (CLegitBot::Get().GetFov() * (diagonal / 145.f)) /** diagonal*/;

		if (Globals::LocalPlayer->m_hActiveWeapon()->IsSniper()) {
			if (Globals::LocalPlayer->m_hActiveWeapon()->m_zoomLevel() == 1) {
				CDraw::Get().Circle(w2, h2, fov * 2.72f, 50, Color::Black);
			}
			else if (Globals::LocalPlayer->m_hActiveWeapon()->m_zoomLevel() == 2) {
				CDraw::Get().Circle(w2, h2, fov * 8.16f, 50, Color::Black);
			}
			else
				CDraw::Get().Circle(w2, h2, fov, 50, Color::Black);
		}
		else
			CDraw::Get().Circle(w2, h2, fov, 50, Color::Black);
	}

}

void CVisuals::Crosshair()
{
	if (!Utils::IsInGame()) 
		return;

	if (!Vars.visuals_crosshair)
		return;

	int MidX = Globals::ScreenWeight / 2; int MidY = Globals::ScreenHeight / 2;
	CDraw::Get().Line(MidX - 8, MidY - 0, MidX + 8, MidY + 0, Color(255, 0, 0, 200));
	CDraw::Get().Line(MidX + 0, MidY - 8, MidX - 0, MidY + 8, Color(255, 0, 0, 200));
	CDraw::Get().Line(MidX - 4, MidY - 0, MidX + 4, MidY + 0, Color(255, 255, 255, 255));
	CDraw::Get().Line(MidX + 0, MidY - 4, MidX - 0, MidY + 4, Color(255, 255, 255, 255));
}

void CVisuals::NoScopeBorder()
{
	if (!Vars.visuals_noscopeborder) return;
	if (!Globals::LocalPlayer) return;
	if (!Globals::LocalPlayer->IsAlive()) return;
	if (!Globals::LocalPlayer->m_hActiveWeapon()) return;
	if (!Globals::LocalPlayer->m_hActiveWeapon()->IsSniper()) return;
	if (!Globals::LocalPlayer->m_bIsScoped()) return;

	float centerX = static_cast<int>(Globals::ScreenWeight * 0.5f);
	float centerY = static_cast<int>(Globals::ScreenHeight * 0.5f);
	Interfaces::Surface->DrawSetColor(Color(0, 0, 0, 255));
	Interfaces::Surface->DrawLine(0, centerY, Globals::ScreenWeight, centerY);
	Interfaces::Surface->DrawLine(centerX, 0, centerX, Globals::ScreenHeight);
}

void CVisuals::PaintTraverse()
{
	CVisuals::RenderESP();
	CVisuals::RenderDrop();
	CVisuals::DrawAimbotFov();
	CVisuals::RenderSpreadXair();
	//CVisuals::Velocity();
	CVisuals::ThirdPerson();
	CVisuals::Crosshair();
	CVisuals::NoScopeBorder();
	CVisuals::Watermark();
}

int getping()
{
	if (!Utils::IsInGame())
		return 0;

	auto nci = Interfaces::Engine->GetNetChannelInfo();
	if (!nci)
		return 0;

	float m_AvgLatency = nci->GetAvgLatency(0);

	return (int)(m_AvgLatency * 1000.0f);
}

int getfps()
{
	return static_cast<int>(1.f / Interfaces::GlobalVars->frametime);
}

void CVisuals::Watermark()
{
	if (!Vars.visuals_watermark) return;

	CDraw::Get().DrawRectFilled(Globals::ScreenWeight - 193, 4, Globals::ScreenWeight - 4, 23, Color(0, 0, 0, 180));
	CDraw::Get().Textf(Globals::ScreenWeight - 187, 5, Color(255, 255, 255, 255), CDraw::Get().font.ESP, XorStr("Nixware | Fps: %03d | Ping: %03d"), Utils::GetFps(), getping());
}

void CVisuals::RenderESP()
{
	if (!Utils::IsInGame())	return;

	for (auto i = 1; i <= Interfaces::EntityList->GetHighestEntityIndex(); ++i)
	{
		C_BasePlayer* entity = (C_BasePlayer*)Interfaces::EntityList->GetClientEntity(i);
		if (!entity)
			continue;

		if (i < 65 && !entity->IsDormant() && entity->IsAlive()) {
			if (Vars.esp_enabled && CVisuals::Begin(entity))
			{
				if (Vars.esp_player_boxes) CVisuals::RenderBox();
				if (Vars.esp_player_health) CVisuals::HealthBar();
				if (Vars.esp_player_names) CVisuals::RenderName();
				if (Vars.esp_player_weapons) CVisuals::RenderWeapon();
			}
		}
	}
}

void CVisuals::Velocity()
{
	if (!Vars.misc_overridefov_inscope)	return;
	if (!Utils::IsInGame())	return;
	if (!Globals::LocalPlayer->IsAlive()) return;

	int maxvel = 1000;

	float flSpeed = Globals::LocalPlayer->m_vecVelocity().Length();

	float headerWidth = flSpeed;
	if (headerWidth > maxvel)
		headerWidth = maxvel;

	CDraw::Get().DrawString(CDraw::Get().font.HitMarker, Globals::ScreenWeight / 2, 780, Color::Green, 2, "Velocity:  %.2f", flSpeed);
	CDraw::Get().DrawHeader(Globals::ScreenWeight / 2 - ((maxvel / 4.f) / 2), 795, maxvel / 4.f, headerWidth / 4.f, Color::Red);
}

void CVisuals::ThirdPerson()
{
	if (!Globals::LocalPlayer) return;

	if (Utils::IsKeyPressed(Vars.visuals_thirdperson_key))
		Globals::ThirdPersponToggle = !Globals::ThirdPersponToggle;

	if (Vars.visuals_thirdperson && Globals::LocalPlayer->IsAlive() && Globals::ThirdPersponToggle) // V Key
	{
		if (!Interfaces::Input->m_fCameraInThirdPerson)
			Interfaces::Input->m_fCameraInThirdPerson = true;
	}
	else
		Interfaces::Input->m_fCameraInThirdPerson = false;
}
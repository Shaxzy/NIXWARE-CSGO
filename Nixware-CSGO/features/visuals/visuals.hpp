#pragma once

#include "../../helpers/singleton.hpp"

#include "../../helpers/draw_manager.hpp"
#include "../../helpers/math.hpp"
#include "../../valve_sdk/csgostructs.hpp"

class CVisuals : public Singleton<CVisuals>
{
public:
	void RenderESP();
	void Velocity();
	void ThirdPerson();
	bool ValidPlayer(C_BasePlayer * player, bool count_step);
	void DrawSkeleton(C_BasePlayer * entity, Color color, const matrix3x4_t * bone_matrix);
	void RenderWeapon(C_BaseCombatWeapon * ent);
	void RenderWeapon();
	void RenderDrop();
	void RenderDefuseKit(C_BaseEntity * ent);
	void RenderPlantedC4(C_BaseEntity * ent);
	bool Begin(C_BasePlayer * pl);
	void RenderBox();
	void HealthBar();
	void RenderName();
	void RenderSpreadXair();
	void DrawAimbotFov();
	void Crosshair();
	void NoScopeBorder();
	void RenderParticles();
	void PaintTraverse();
	void Watermark();
};
extern bool hardDisable;
class CHudModulation : public Singleton<CHudModulation>
{
public:
	void FrameStageNotify(ClientFrameStage_t stage);
	void ToggleVGuiModulation(bool enabled);
};

void obrabotka();

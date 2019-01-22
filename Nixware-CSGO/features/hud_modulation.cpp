#include "visuals/visuals.hpp"

float r, g, b, a;
bool sprop = false;

static int frameCounter = 0;
struct ModColor
{
	float color[3];
	float alpha;

	void SetColorFloat(float r, float g, float b)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
	}

	void SetColorFloat(float col[3])
	{
		color[0] = col[0];
		color[1] = col[1];
		color[2] = col[2];
	}

	void SetColor(Color col)
	{
		color[0] = col.r() / 255.f;
		color[1] = col.g() / 255.f;
		color[2] = col.b() / 255.f;
	}

	void GetColorFloat(float col[3])
	{
		col[0] = color[0];
		col[1] = color[1];
		col[2] = color[2];
	}

	Color GetColor()
	{
		return Color(color[0], color[1], color[2]);
	}

	ModColor(float r, float g, float b, float alph)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		alpha = alph;
	}

	ModColor(float col[3], float alph)
	{
		color[0] = col[0];
		color[1] = col[1];
		color[2] = col[2];
		alpha = alph;
	}
};
std::unordered_map<MaterialHandle_t, std::pair<ModColor, ModColor>> vguiMaterials;
bool hardDisable = false;

void CHudModulation::FrameStageNotify(ClientFrameStage_t stage)
{
	if (hardDisable || stage != ClientFrameStage_t::FRAME_RENDER_START || frameCounter++ < 20)
		return;

	frameCounter = 0;

	for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
	{
		IMaterial* mat = Interfaces::MaterialSystem->GetMaterial(i);

		Color originalColor = Color(255, 255, 255, 255);
		Color vguiColor = Color(81, 81, 81, 210);

		//Check if a VGUI material
		if (mat && (strcmp(mat->GetName(), XorStr("vgui_white")) == 0 || strstr(mat->GetName(), XorStr("vgui/hud/800corner")) != NULL)) {
			//Not yet in the list
			if (vguiMaterials.find(i) == vguiMaterials.end())
			{
				mat->GetColorModulation(&r, &g, &b);
				originalColor = Color(r, g, b, mat->GetAlphaModulation() * 255);
				vguiMaterials.emplace(i, std::make_pair(ModColor(1, 1, 1, 1.f), ModColor(1, 1, 1, vguiColor.a() / 255.f)));
			}
			//Color has been changed
			if (vguiMaterials.at(i).second.GetColor() != vguiColor || vguiMaterials.at(i).second.alpha != vguiColor.a() / 255.f)
			{
				//0.00392156862f = 1 / 255
				mat->ColorModulate(0.00392156862f * vguiColor[0], 0.00392156862f * vguiColor[1], 0.00392156862f * vguiColor[2]);
				mat->AlphaModulate(vguiColor.a() / 255.f);
				vguiMaterials.at(i).second.SetColor(vguiColor);
			}
		}
	}
}

void CHudModulation::ToggleVGuiModulation(bool enable)
{
	if (!hardDisable && enable)
	{
		for (const auto& it : vguiMaterials)
		{
			IMaterial* mat = Interfaces::MaterialSystem->GetMaterial(it.first);

			if (!mat)
				continue;

			mat->ColorModulate(it.second.second.color[0], it.second.second.color[1], it.second.second.color[2]);
			mat->AlphaModulate(it.second.second.alpha);
		}
	}
	else
	{
		for (const auto& it : vguiMaterials)
		{
			IMaterial* mat = Interfaces::MaterialSystem->GetMaterial(it.first);

			if (!mat)
				continue;

			mat->ColorModulate(1, 1, 1);
			mat->AlphaModulate(1);
		}
	}
}
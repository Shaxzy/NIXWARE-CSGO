#pragma once

#include <string>
#include "../helpers/singleton.hpp"
#include "../imgui/imgui.h"

#include <D3dx9tex.h>
#pragma comment (lib,"D3dx9.lib")

struct IDirect3DDevice9;

class CMenu : public Singleton<CMenu>
{
public:
	IDirect3DTexture9 *anime_picture;

	void InvalidateDeviceObjects();
	void CreateDeviceObjects();
	void EndScene(IDirect3DDevice9 * device);
	void Shutdown();
private:
	void PlayerListFrame();
	void RenderLegitbotTab();
	void RenderSkinsTab();
	void RenderRagebotTab();
	void RenderVisualsTab();
	void RenderMiscTab();
	void MainFrame();
	void RadioFrame();
	void SpectatorListFrame();
	bool Initialize(IDirect3DDevice9 * device);
	void ImColors(ImGuiStyle & style);
	void ImStyles(ImGuiStyle & style);
};
#pragma once
#include "xor.h"

//#define ENABLE_XOR

#ifdef ENABLE_XOR
#define XorStr _xor_ 
#else
#define XorStr
#endif

#define IP "35.204.191.243"
#define PORT "1337"

#define I Interfaces
#define G Globals
#define U Utils
#define M Math

#define INTERFACES Interfaces
#define UTILS Utils
#define MATH Math

#define g_EngineClient Interfaces::Engine
#define g_CHLClient Interfaces::Client
#define g_EntityList Interfaces::EntityList
#define g_GlobalVars Interfaces::GlobalVars
#define g_EngineTrace Interfaces::EngineTrace
#define g_CVar Interfaces::Convar
#define g_VGuiPanel Interfaces::Panel
#define g_ClientMode Interfaces::ClientMode
#define g_DebugOverlay Interfaces::DebugOverlay
#define g_VGuiSurface Interfaces::Surface
#define g_Input Interfaces::Input
#define g_MdlInfo Interfaces::ModelInfo
#define g_MdlRender Interfaces::ModelRender
#define g_RenderView Interfaces::RenderView
#define g_MatSystem Interfaces::MaterialSystem
#define g_GameEvents Interfaces::GameEvents
#define g_MoveHelper Interfaces::MoveHelper
#define g_MdlCache Interfaces::MdlCache
#define g_Prediction Interfaces::Prediction
#define g_GameMovement Interfaces::GameMovement
#define g_EngineSound Interfaces::EngineSound
#define g_GlowObjManager Interfaces::GlowManager
#define g_ViewRender Interfaces::ViewRender
#define g_D3DDevice9 Interfaces::D3DDevice
#define g_ClientState Interfaces::ClientState
#define g_PhysSurface Interfaces::PhysSurface

#define g_LocalPlayer Globals::LocalPlayer

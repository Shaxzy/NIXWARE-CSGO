#pragma once

#include "valve_sdk/csgostructs.hpp"
#include "helpers/vfunc_hook.hpp"
#include <d3d9.h>

namespace index
{
	constexpr auto EmitSound1               = 5;
	constexpr auto EmitSound2               = 6;
    constexpr auto EndScene                 = 42;
    constexpr auto Reset                    = 16;
	constexpr auto SceneEnd                 = 9;
    constexpr auto PaintTraverse            = 41;
    constexpr auto CreateMove               = 24;
	constexpr auto DispatchUserMessage      = 38;
	constexpr auto NetGraph                 = 13;
	constexpr auto SendDatagram             = 46;
    constexpr auto PlaySound_               = 82;
    constexpr auto FrameStageNotify         = 37;
	constexpr auto FireEventClientSide      = 9;
    constexpr auto DrawModelExecute         = 21;
	constexpr auto SvCheatsGetBool          = 13;
	constexpr auto OverrideView             = 18;
	constexpr auto ViewModel                = 35;
	constexpr auto LockCursor               = 67;
}

namespace Hooks
{
    void Initialize();
    void Unload();

	extern vfunc_hook hlclient_hook;
	extern vfunc_hook event_hook;
	extern vfunc_hook direct3d_hook;
	extern vfunc_hook vguipanel_hook;
	extern vfunc_hook vguisurf_hook;
	extern vfunc_hook sound_hook;
	extern vfunc_hook mdlrender_hook;
	extern vfunc_hook renderview_hook;
	extern vfunc_hook clientmode_hook;
	extern vfunc_hook sv_cheats;
	extern vfunc_hook netgraphtext_hook;
	extern vfunc_hook net_channel_hook_manager;
	extern vfunc_hook FindOrAddFileName_hook;

	typedef bool(__thiscall *CreateMove_t)(IClientMode*, float, CUserCmd*);
    using EndScene            = long(__stdcall *)(IDirect3DDevice9*);
    using Reset               = long(__stdcall *)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
    using CreateMove          = void(__thiscall*)(IBaseClientDLL*, int, float, bool);
	using SendDatagram_t      = int(__thiscall *)(void *, void *);
    using PaintTraverse       = void(__thiscall*)(IPanel*, vgui::VPANEL, bool, bool);
	using EmitSound1          = void(__thiscall*)(void*, IRecipientFilter&, int, int, const char*, unsigned int, const char*, float, int, float, int, int, const Vector*, const Vector*, void*, bool, float, int, int);
    using FrameStageNotify    = void(__thiscall*)(IBaseClientDLL*, ClientFrameStage_t);
    using PlaySound_           = void(__thiscall*)(ISurface*, const char* name);
	using LockCursor_t        = void(__thiscall*)(ISurface*);
    using DrawModelExecute    = void(__thiscall*)(IVModelRender*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
    using FireEvent           = bool(__thiscall*)(IGameEventManager2*, IGameEvent* pEvent);
	using OverrideView        = void(__thiscall*)(IClientMode*, CViewSetup*);
	using ViewModelFov        = float(__stdcall*)();

	using iSceneEnd           = void(__fastcall*)(void*, void*);
	using netgraphtextFn      = bool(__thiscall*)(void*);
	using iClientCmd          = void(__fastcall*)(void*, const char*);
	using FireEventClientSide = bool(__thiscall*)(void*, IGameEvent*);

	typedef bool(__thiscall *tDispatchUserMessage)(void*, int type, unsigned int a3, unsigned int length, const void *msg_data);
	//typedef void(__fastcall *iClientCmd)(void*, void*, const char*);

	extern SendDatagram_t oSendDatagram;

	bool __fastcall hkNetGraph(void * ecx, void * edx);
	bool __fastcall hkDispatchUserMessage(void * ecx, void * edx, int type, unsigned int a3, unsigned int length, const void * msg_data);
	long __stdcall hkEndScene(IDirect3DDevice9* device);
    long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
	int __fastcall hkSendDatagram(void * net_channel, void *, void * datagram);
	void __fastcall hkSceneEnd(void* thisptr, void* edx);
	bool __stdcall hkCreateMove(float smt, CUserCmd * cmd);
	void __stdcall hkPaintTraverse(vgui::VPANEL panel, bool forceRepaint, bool allowForce);
	void __stdcall hkEmitSound(IRecipientFilter & filter, int iEntIndex, int iChannel, const char * pSoundEntry, unsigned int nSoundEntryHash, const char * pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector * pOrigin, const Vector * pDirection, void * pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk);
    void __stdcall hkPlaySound(const char* name);
    void __stdcall hkDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
    void __stdcall hkFrameStageNotify(ClientFrameStage_t stage);
	bool __fastcall hkFireEventClientSideThink(void * ecx, void * edx, IGameEvent * pEvent);
	float __stdcall hkViewModelFov();
	void __stdcall hkOverrideView(CViewSetup * vsView);
	void __stdcall hkLockCursor();
    int  __stdcall hkDoPostScreenEffects(int a1);
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx);
}

#include "sdk.hpp"

#include "../Helpers/Utils.hpp"
#define PRINT_INTERFACE(name) Utils::ConsolePrint(XorStr("%-20s: %p\n"), #name, name)

namespace Interfaces
{
	IVEngineClient*       Engine = nullptr;
	IBaseClientDLL*       Client = nullptr;
	IGameTypes*           GameType = nullptr;
	IClientEntityList*    EntityList = nullptr;
	CGlobalVarsBase*      GlobalVars = nullptr;
	IEngineTrace*         EngineTrace = nullptr;
	ICvar*                Convar = nullptr;
	IPanel*               Panel = nullptr;
	IClientMode*          ClientMode = nullptr;
	IVDebugOverlay*       DebugOverlay = nullptr;
	ISurface*             Surface = nullptr;
	IFileSystem*          FileSystem = nullptr;
	CInput*               Input = nullptr;
	IVModelInfoClient*    ModelInfo = nullptr;
	IVModelRender*        ModelRender = nullptr;
	IVRenderView*         RenderView = nullptr;
	IMaterialSystem*      MaterialSystem = nullptr;
	IGameEventManager2*   GameEvents = nullptr;
	IMoveHelper*          MoveHelper = nullptr;
	IMDLCache*            MdlCache = nullptr;
	IPrediction*          Prediction = nullptr;
	CGameMovement*        GameMovement = nullptr;
	IEngineSound*         EngineSound = nullptr;
	CGlowObjectManager*   GlowManager = nullptr;
	IViewRender*          ViewRender = nullptr;
	IDirect3DDevice9*     D3DDevice = nullptr;
	CClientState*         ClientState = nullptr;
	IPhysicsSurfaceProps* PhysSurface = nullptr;
	ILocalize*            Localize = nullptr;
}

namespace Globals
{
	int ScreenWeight;
	int ScreenHeight;

	C_LocalPlayer         LocalPlayer;
}

namespace Interfaces
{
	CreateInterfaceFn GetModuleFactory(HMODULE module)
	{
		return reinterpret_cast<CreateInterfaceFn>(GetProcAddress(module, "CreateInterface"));
	}

	template<typename T>
	T* GetInterface(CreateInterfaceFn f, const char* szInterfaceVersion)
	{
		auto result = reinterpret_cast<T*>(f(szInterfaceVersion, nullptr));

		if (!result) {
			throw std::runtime_error(std::string("[get_interface] Failed to GetOffset interface: ") + szInterfaceVersion);
		}

		return result;
	}

	void Initialize()
	{
		auto EngineFactory = GetModuleFactory(GetModuleHandleA(XorStr("engine.dll")));
		auto MatchmakingFactory = GetModuleFactory(GetModuleHandleA(XorStr("matchmaking.dll")));
		auto FileSystemFactory = GetModuleFactory(GetModuleHandleA(XorStr("filesystem_stdio.dll")));
		auto ClientFactory = GetModuleFactory(GetModuleHandleA(XorStr("client_panorama.dll")));
		auto ValveStdFactory = GetModuleFactory(GetModuleHandleA(XorStr("vstdlib.dll")));
		auto VguiFactory = GetModuleFactory(GetModuleHandleA(XorStr("vguimatsurface.dll")));
		auto Vgui2Factory = GetModuleFactory(GetModuleHandleA(XorStr("vgui2.dll")));
		auto MatSysFactory = GetModuleFactory(GetModuleHandleA(XorStr("materialsystem.dll")));
		auto DataCacheFactory = GetModuleFactory(GetModuleHandleA(XorStr("datacache.dll")));
		auto VphysicsFactory = GetModuleFactory(GetModuleHandleA(XorStr("vphysics.dll")));
		auto LocalizeFactory = GetModuleFactory(GetModuleHandleA(XorStr("localize.dll")));

		auto Client = GetModuleHandleA(XorStr("client_panorama.dll"));
		auto Engine = GetModuleHandleA(XorStr("engine.dll"));
		auto DX9api = GetModuleHandleA(XorStr("shaderapidx9.dll"));

		Interfaces::Client = GetInterface<IBaseClientDLL>(ClientFactory, XorStr("VClient018"));
		Interfaces::EntityList = GetInterface<IClientEntityList>(ClientFactory, XorStr("VClientEntityList003"));
		Interfaces::Prediction = GetInterface<IPrediction>(ClientFactory, XorStr("VClientPrediction001"));
		Interfaces::GameMovement = GetInterface<CGameMovement>(ClientFactory, XorStr("GameMovement001"));
		Interfaces::MdlCache = GetInterface<IMDLCache>(DataCacheFactory, XorStr("MDLCache004"));
		Interfaces::Engine = GetInterface<IVEngineClient>(EngineFactory, XorStr("VEngineClient014"));
		Interfaces::GameType = GetInterface<IGameTypes>(MatchmakingFactory, XorStr("VENGINE_GAMETYPES_VERSION002"));
		Interfaces::ModelInfo = GetInterface<IVModelInfoClient>(EngineFactory, XorStr("VModelInfoClient004"));
		Interfaces::ModelRender = GetInterface<IVModelRender>(EngineFactory, XorStr("VEngineModel016"));
		Interfaces::RenderView = GetInterface<IVRenderView>(EngineFactory, XorStr("VEngineRenderView014"));
		Interfaces::EngineTrace = GetInterface<IEngineTrace>(EngineFactory, XorStr("EngineTraceClient004"));
		Interfaces::DebugOverlay = GetInterface<IVDebugOverlay>(EngineFactory, XorStr("VDebugOverlay004"));
		Interfaces::GameEvents = GetInterface<IGameEventManager2>(EngineFactory, XorStr("GAMEEVENTSMANAGER002"));
		Interfaces::EngineSound = GetInterface<IEngineSound>(EngineFactory, XorStr("IEngineSoundClient003"));
		Interfaces::MaterialSystem = GetInterface<IMaterialSystem>(MatSysFactory, XorStr("VMaterialSystem080"));
		Interfaces::Convar = GetInterface<ICvar>(ValveStdFactory, XorStr("VEngineCvar007"));
		Interfaces::Panel = GetInterface<IPanel>(Vgui2Factory, XorStr("VGUI_Panel009"));
		Interfaces::Surface = GetInterface<ISurface>(VguiFactory, XorStr("VGUI_Surface031"));
		Interfaces::PhysSurface = GetInterface<IPhysicsSurfaceProps>(VphysicsFactory, XorStr("VPhysicsSurfaceProps001"));
		Interfaces::Localize = GetInterface<ILocalize>(LocalizeFactory, XorStr("Localize_001"));
		Interfaces::FileSystem = GetInterface<IFileSystem>(FileSystemFactory, XorStr("VBaseFileSystem011"));

		Interfaces::GlobalVars = **(CGlobalVarsBase***)(Utils::PatternScan(Client, XorStr("A1 ? ? ? ? 5E 8B 40 10")) + 1);
		Interfaces::ClientMode = *(IClientMode**)(Utils::PatternScan(Client, XorStr("A1 ? ? ? ? 8B 80 ? ? ? ? 5D")) + 1);
		Interfaces::Input = *(CInput**)(Utils::PatternScan(Client, XorStr("B9 ? ? ? ? 8B 40 38 FF D0 84 C0 0F 85")) + 1);
		Interfaces::MoveHelper = **(IMoveHelper***)(Utils::PatternScan(Client, XorStr("8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01")) + 2);
		Interfaces::GlowManager = *(CGlowObjectManager**)(Utils::PatternScan(Client, XorStr("0F 11 05 ? ? ? ? 83 C8 01")) + 3);
		Interfaces::ViewRender = **reinterpret_cast<IViewRender***>(Utils::PatternScan(Client, "FF 50 4C 8B 06 8D 4D F4") - 6);
		Interfaces::D3DDevice = **(IDirect3DDevice9***)(Utils::PatternScan(DX9api, XorStr("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 1);

		Interfaces::ClientState = **(CClientState***)(Utils::PatternScan(Engine, XorStr("A1 ? ? ? ? 8B 80 ? ? ? ? C3")) + 1);

		Globals::LocalPlayer = *(C_LocalPlayer*)(Utils::PatternScan(Client, XorStr("8B 0D ? ? ? ? 83 FF FF 74 07")) + 2);
	}

	void Dump()
	{
		PRINT_INTERFACE(Interfaces::Client);
		PRINT_INTERFACE(Interfaces::EntityList);
		PRINT_INTERFACE(Interfaces::Prediction);
		PRINT_INTERFACE(Interfaces::GameMovement);
		PRINT_INTERFACE(Interfaces::MdlCache);
		PRINT_INTERFACE(Interfaces::Engine);
		PRINT_INTERFACE(Interfaces::ModelInfo);
		PRINT_INTERFACE(Interfaces::ModelRender);
		PRINT_INTERFACE(Interfaces::RenderView);
		PRINT_INTERFACE(Interfaces::EngineTrace);
		PRINT_INTERFACE(Interfaces::DebugOverlay);
		PRINT_INTERFACE(Interfaces::GameEvents);
		PRINT_INTERFACE(Interfaces::EngineSound);
		PRINT_INTERFACE(Interfaces::MaterialSystem);
		PRINT_INTERFACE(Interfaces::Convar);
		PRINT_INTERFACE(Interfaces::Panel);
		PRINT_INTERFACE(Interfaces::Surface);
		PRINT_INTERFACE(Interfaces::PhysSurface);
		PRINT_INTERFACE(Interfaces::FileSystem);
		PRINT_INTERFACE(Interfaces::Localize);

	}
}
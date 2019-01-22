#define NOMINMAX
#include "security/security.h"
#include <Windows.h>

#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/config_manager.hpp"
#include "helpers/input.hpp"
#ifndef _DEBUG
#endif // !_DEBUG

#include "features/skinchanger/skins.hpp"

#include "hooks.hpp"
#include "menu/menu.hpp"
#include "options.hpp"

#include <TlHelp32.h>

static unsigned char global_opCodes[] = { 0x8b, 0xff, 0x55, 0x8b, 0xec };
static unsigned char readfile_opCodes[] = { 0xff, 0x25, 0x30, 0x04, 0x91 };

auto CheckHook = [](PDWORD pdwAddress, unsigned char* opCodes)
{
	for (int i = 0; i < 5; i++)
	{
		if (((unsigned char*)pdwAddress)[i] != opCodes[i]) *((unsigned int*)0) = 0xDEAD;//MessageBox(0, "Hook detected", 0, 0);
	}
};

DWORD WINAPI CheckHooks(PVOID base)
{
	while (true)
	{
		CheckHook((PDWORD)getaddrinfo, global_opCodes);
		CheckHook((PDWORD)recv, global_opCodes);
		CheckHook((PDWORD)send, global_opCodes);
		//CheckHook((PDWORD)ReadFile, readfile_opCodes);
		Sleep(500);
	}
}

DWORD WINAPI OnDllAttach(PVOID base)
{
	VMProtectBeginUltra("DllAttach");

	while (!GetModuleHandleA(XorStr("serverbrowser.dll")))
		Sleep(1000);
  	
#ifndef _DEBUG
	if (CSecurity::Get().Connect())
		Beep(659.26, 300);

	CSecurity::Get().LoadConfig();
	CSecurity::Get().Auth();
#endif // !_DEBUG

    try {
		//Utils::AttachConsole();

        Utils::ConsolePrint(XorStr("Initializing...\n"));

        Interfaces::Initialize();
        Interfaces::Dump();

        NetvarSys::Get().Initialize();
        InputSys::Get().Initialize();
		CConfig::Get().Initialize();
		CSkinChanger::Get().Initialize();

        Hooks::Initialize();

        Utils::ConsolePrint(XorStr("Finished.\n"));

        while (!Globals::Unload)
            Sleep(1000);

		Hooks::Unload();

		FreeLibraryAndExitThread(HMODULE(base), 0);

    } catch(const std::exception& ex) {
        Utils::ConsolePrint(XorStr("An error occured during initialization:\n"));
        Utils::ConsolePrint(XorStr("%s\n"), ex.what());
        Utils::ConsolePrint(XorStr("Press any key to exit.\n"));
        Utils::ConsoleReadKey();
        Utils::DetachConsole();

		FreeLibraryAndExitThread(HMODULE(base), 0);
	}
	VMProtectEnd();
}

BOOL WINAPI OnDllDetach()
{
    Utils::DetachConsole();

    Hooks::Unload();

    return TRUE;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDll, _In_ DWORD fdwReason, _In_opt_ LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDll);
		CheckHook((PDWORD)CreateThread, global_opCodes);
		CreateThread(nullptr, 0, CheckHooks, hinstDll, 0, nullptr);
		CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);

		return TRUE;
	}
}

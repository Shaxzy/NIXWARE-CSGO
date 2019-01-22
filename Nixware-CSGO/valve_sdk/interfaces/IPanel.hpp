#pragma once

class IPanel
{
public:
    const char *GetName(unsigned int vguiPanel)
    {
        typedef const char *(__thiscall* OriginalFn)(PVOID, unsigned int);
        return CallVFunction<OriginalFn>(this, 36)(this, vguiPanel);
    }
	void SetMouseInputEnabled(vgui::VPANEL vguiPanel, bool state)
	{
		typedef void(__thiscall* OriginalFn)(PVOID, vgui::VPANEL, bool);
		return CallVFunction< OriginalFn >(this, 32)(this, vguiPanel, state);
	}
#ifdef GetClassName
#undef GetClassName
#endif
    const char *GetClassName(unsigned int vguiPanel)
    {
        typedef const char *(__thiscall* tGetClassName)(PVOID, unsigned int);
        return CallVFunction<tGetClassName>(this, 37)(this, vguiPanel);
    }
};

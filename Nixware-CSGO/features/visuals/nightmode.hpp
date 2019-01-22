#pragma once
#include "../../options.hpp"

class CNightmode : public Singleton<CNightmode>
{
public:
	std::string OldSkyname = "";
	bool NightmodeDone = true;
	void UpdateWorldTextures();
	void PerformNightmode();
};

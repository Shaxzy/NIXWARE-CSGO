#pragma once
#include "../../valve_sdk/csgostructs.hpp"

class CChams : public Singleton<CChams>
{
public: 
	IMaterial* material_norm;
	void SceneEnd();
	void Initialize();
	void Shutdown();
private:
	bool TeamCheck(C_BasePlayer * ent);
};

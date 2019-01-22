#pragma once

#include <string>
#include <vector>
#include <Windows.h>
#include "utils.hpp"

template <typename T>
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_)
	{
		category = category_;
		name = name_;
		value = value_;
	}

	std::string category, name;
	T* value;
};

class CConfig : public Singleton<CConfig>
{
protected:
	std::vector<ConfigValue<int>*> ints;
	std::vector<ConfigValue<bool>*> bools;
	std::vector<ConfigValue<float>*> floats;
private:
	void SetupValue(int & value, int def, std::string category, std::string name);
	void SetupValue(bool & value, bool def, std::string category, std::string name);
	void SetupValue(float & value, float def, std::string category, std::string name);
	void SetupColor(float value[4], std::string name);
	void SetupLegit();
	void SetupRage();
	void SetupSkinchanger();
	void SetupMisc();
	void SetupVisuals();
	void SetupColors();
public:
	void Initialize();
	void Load(std::string szIniFile);
	void Save(std::string szIniFile);
};
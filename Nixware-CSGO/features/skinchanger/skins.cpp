#include "skins.hpp"

static auto erase_override_if_exists_by_index(const int definition_index) -> void
{
	if (k_weapon_info.count(definition_index))
	{
		auto& icon_override_map = Vars.skins.m_icon_overrides;
		const auto& original_item = k_weapon_info.at(definition_index);
		if (original_item.icon && icon_override_map.count(original_item.icon))
			icon_override_map.erase(icon_override_map.at(original_item.icon)); // Remove the leftover override
	}
}
static auto apply_config_on_attributable_item(C_BaseAttributableItem* item, const CVariables::weapon_settings* config,
	const unsigned xuid_low) -> void
{
	if (!config->enabled) {
		return;
	}
	item->m_Item().m_iItemIDHigh() = -1;
	item->m_Item().m_iAccountID() = xuid_low;
	if (config->custom_name[0])
		strcpy_s(item->m_Item().m_iCustomName(), config->custom_name);
	if (config->paint_kit_index)
		item->m_nFallbackPaintKit() = config->paint_kit_index;
	if (config->seed)
		item->m_nFallbackSeed() = config->seed;
	if (config->stat_trak) {
		item->m_nFallbackStatTrak() = config->stat_trak;
		item->m_Item().m_iEntityQuality() = 9;
	}
	else {
		item->m_Item().m_iEntityQuality() = is_knife(config->definition_index) ? 3 : 0;
	}
	item->m_flFallbackWear() = config->wear;
	auto& definition_index = item->m_Item().m_iItemDefinitionIndex();
	auto& icon_override_map = Vars.skins.m_icon_overrides;
	if (config->definition_override_index && config->definition_override_index != definition_index && k_weapon_info.count(config->definition_override_index))
	{
		const auto old_definition_index = definition_index;
		definition_index = config->definition_override_index;
		const auto& replacement_item = k_weapon_info.at(config->definition_override_index);
		item->m_nModelIndex() = g_MdlInfo->GetModelIndex(replacement_item.model);
		item->SetModelIndex(g_MdlInfo->GetModelIndex(replacement_item.model));
		item->GetClientNetworkable()->PreDataUpdate(0);
		if (old_definition_index && k_weapon_info.count(old_definition_index))
		{
			const auto& original_item = k_weapon_info.at(old_definition_index);
			if (original_item.icon && replacement_item.icon) {
				icon_override_map[original_item.icon] = replacement_item.icon;
			}
		}
	}
	else
	{
		erase_override_if_exists_by_index(definition_index);
	}
}
static auto get_wearable_create_fn() -> CreateClientClassFn
{
	auto clazz = Interfaces::Client->GetAllClasses();
	while (strcmp(clazz->m_pNetworkName, "CEconWearable"))
		clazz = clazz->m_pNext;
	return clazz->m_pCreateFn;
}

int CSkinChanger::GetWeaponSettingsSelectID_ForSkins(C_BaseAttributableItem* weapon)
{
	if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_KNIFE)
		return WEAPON_KNIFE;

	if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_KNIFE_T)
		return WEAPON_KNIFE_T;

	for (size_t i = 0; i < (sizeof(Utils::pWeaponData) / sizeof(*Utils::pWeaponData)); i++)
	{
		if (Utils::pWeaponItemIndexData[i] == weapon->m_Item().m_iItemDefinitionIndex())
			return i;
	}
}

RecvVarProxyFn fnSequenceProxyFn = NULL;

#define SEQUENCE_DEFAULT_DRAW 0
#define SEQUENCE_DEFAULT_IDLE1 1
#define SEQUENCE_DEFAULT_IDLE2 2
#define SEQUENCE_DEFAULT_LIGHT_MISS1 3
#define SEQUENCE_DEFAULT_LIGHT_MISS2 4
#define SEQUENCE_DEFAULT_HEAVY_MISS1 9
#define SEQUENCE_DEFAULT_HEAVY_HIT1 10
#define SEQUENCE_DEFAULT_HEAVY_BACKSTAB 11
#define SEQUENCE_DEFAULT_LOOKAT01 12

#define SEQUENCE_BUTTERFLY_DRAW 0
#define SEQUENCE_BUTTERFLY_DRAW2 1
#define SEQUENCE_BUTTERFLY_LOOKAT01 13
#define SEQUENCE_BUTTERFLY_LOOKAT03 15

#define SEQUENCE_FALCHION_IDLE1 1
#define SEQUENCE_FALCHION_HEAVY_MISS1 8
#define SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP 9
#define SEQUENCE_FALCHION_LOOKAT01 12
#define SEQUENCE_FALCHION_LOOKAT02 13

#define SEQUENCE_DAGGERS_IDLE1 1
#define SEQUENCE_DAGGERS_LIGHT_MISS1 2
#define SEQUENCE_DAGGERS_LIGHT_MISS5 6
#define SEQUENCE_DAGGERS_HEAVY_MISS2 11
#define SEQUENCE_DAGGERS_HEAVY_MISS1 12

#define SEQUENCE_BOWIE_IDLE1 1
#define RandomInt(min, max) (rand() % (max - min + 1) + min)

void Hook_SetViewModelSequence(const CRecvProxyData *pDataConst, void *pStruct, void *pOut)
{
	CRecvProxyData* pData = const_cast<CRecvProxyData*>(pDataConst);
	C_BaseViewModel* pViewModel = (C_BaseViewModel*)pStruct;

	if (pViewModel)
	{
		IClientEntity* pOwner = Interfaces::EntityList->GetClientEntityFromHandle(pViewModel->m_hOwner());

		if (pOwner && pOwner->EntIndex() == Interfaces::Engine->GetLocalPlayer())
		{
			const model_t* pModel = pViewModel->GetModel();
			const char* szModel = Interfaces::ModelInfo->GetModelName(pModel);
            #define f(s) !strcmp(szModel, s)
			int m_nSequence = pData->m_Value.m_Int;
			if (f("models/weapons/v_knife_butterfly.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_DRAW:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2); break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03); break;
				default:
					m_nSequence++;
				}
			}
			else if (f("models/weapons/v_knife_falchion_advanced.mdl"))
			{
				// Fix animations for the Falchion Knife.
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_FALCHION_IDLE1; break;
				case SEQUENCE_DEFAULT_HEAVY_MISS1:
					m_nSequence = RandomInt(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP); break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02); break;
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				default:
					m_nSequence--;
				}
			}
			else if (f("models/weapons/v_knife_push.mdl"))
			{
				// Fix animations for the Shadow Daggers.
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_DAGGERS_IDLE1; break;
				case SEQUENCE_DEFAULT_LIGHT_MISS1:
				case SEQUENCE_DEFAULT_LIGHT_MISS2:
					m_nSequence = RandomInt(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5); break;
				case SEQUENCE_DEFAULT_HEAVY_MISS1:
					m_nSequence = RandomInt(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1); break;
				case SEQUENCE_DEFAULT_HEAVY_HIT1:
				case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence += 3; break;
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				default:
					m_nSequence += 2;
				}
			}
			else if (f("models/weapons/v_knife_survival_bowie.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_BOWIE_IDLE1; break;
				default:
					m_nSequence--;
				}
			}
			else if (f("models/weapons/v_knife_ursus.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_DRAW:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
				default:
					m_nSequence = m_nSequence + 1;
				}
			}
			else if (f("models/weapons/v_knife_stiletto.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(12, 13);
				}
			}
			else if (f("models/weapons/v_knife_widowmaker.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(14, 15);
				}
			}
			pData->m_Value.m_Int = m_nSequence;
		}
	}

	fnSequenceProxyFn(pData, pStruct, pOut);
}

void CSkinChanger::HookSequence()
{
	for (ClientClass* pClass = Interfaces::Client->GetAllClasses(); pClass; pClass = pClass->m_pNext)
	{
		if (!strcmp(pClass->m_pNetworkName, "CBaseViewModel"))
		{
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++)
			{
				RecvProp* pProp = &pClassTable->m_pProps[nIndex];

				if (!pProp || strcmp(pProp->m_pVarName, "m_nSequence"))
					continue;

				fnSequenceProxyFn = pProp->m_ProxyFn;
				pProp->m_ProxyFn = (RecvVarProxyFn)Hook_SetViewModelSequence;

				break;
			}

			break;
		}
	}
}

void CSkinChanger::Update()
{
	if (k_skins.empty())
		return;

	Vars.weapon[5028].definition_override_index = k_glove_names.at(Vars.weapon[5028].definition_override_vector_index).definition_index;
	Vars.weapon[5028].paint_kit_index = k_gloves[Vars.weapon[5028].paint_kit_vector_index].id;

	Vars.weapon[WEAPON_KNIFE_T].paint_kit_index = k_skins[Vars.weapon[WEAPON_KNIFE_T].paint_kit_vector_index].id;
	Vars.weapon[WEAPON_KNIFE_T].definition_override_index = k_knife_names.at(Vars.weapon[WEAPON_KNIFE_T].definition_override_vector_index).definition_index;

	Vars.weapon[WEAPON_KNIFE].paint_kit_index = k_skins[Vars.weapon[WEAPON_KNIFE].paint_kit_vector_index].id;
	Vars.weapon[WEAPON_KNIFE].definition_override_index = k_knife_names.at(Vars.weapon[WEAPON_KNIFE].definition_override_vector_index).definition_index;

	for (DWORD i = 0; i < (sizeof(Utils::pWeaponData) / sizeof(*Utils::pWeaponData)); i++)
	{
		Vars.weapon[i].paint_kit_index = k_skins[Vars.weapon[i].paint_kit_vector_index].id;
	}

	Utils::ForceFullUpdate();
}

void CSkinChanger::OnFrameStageNotify(ClientFrameStage_t stage)
{
	if (!Utils::IsInGame())
		return;

	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	const auto local_index = Interfaces::Engine->GetLocalPlayer();
	//const auto local = static_cast<C_BasePlayer*>(Interfaces::EntityList->GetClientEntity(local_index));
	auto local = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());

	if (!local)
		return;

	player_info_t player_info;
	if (!Interfaces::Engine->GetPlayerInfo(local_index, &player_info))
		return;
	{
		const auto wearables = local->m_hMyWearables();
		const auto glove_config = &Vars.weapon[5028];
		static auto glove_handle = CBaseHandle(0);
		auto glove = reinterpret_cast<C_BaseAttributableItem*>(Interfaces::EntityList->GetClientEntityFromHandle(wearables[0]));
		if (!glove)
		{
			const auto our_glove = reinterpret_cast<C_BaseAttributableItem*>(Interfaces::EntityList->GetClientEntityFromHandle(glove_handle));
			if (our_glove) // Our glove still exists
			{
				wearables[0] = glove_handle;
				glove = our_glove;
			}
		}
		if (!local->IsAlive())
		{
			if (glove)
			{
				glove->GetClientNetworkable()->SetDestroyedOnRecreateEntities();
				glove->GetClientNetworkable()->Release();
			}
			return;
		}
		if (glove_config && glove_config->definition_override_index)
		{
			if (!glove)
			{
				static auto create_wearable_fn = get_wearable_create_fn();
				const auto entry = Interfaces::EntityList->GetHighestEntityIndex() + 1;
				const auto serial = rand() % 0x1000;
				//glove = static_cast<C_BaseAttributableItem*>(create_wearable_fn(entry, serial));
				create_wearable_fn(entry, serial);
				glove = reinterpret_cast<C_BaseAttributableItem*>(Interfaces::EntityList->GetClientEntity(entry));
				assert(glove);
				{
					static auto set_abs_origin_addr = Utils::PatternScan(GetModuleHandleA(XorStr("client_panorama.dll")), XorStr("55 8B EC 83 E4 F8 51 53 56 57 8B F1"));
					const auto set_abs_origin_fn = reinterpret_cast<void(__thiscall*)(void*, const std::array<float, 3>&)>(set_abs_origin_addr);
					static constexpr std::array<float, 3> new_pos = { 10000.f, 10000.f, 10000.f };
					set_abs_origin_fn(glove, new_pos);
				}
				const auto wearable_handle = reinterpret_cast<CBaseHandle*>(&wearables[0]);
				*wearable_handle = entry | serial << 16;
				glove_handle = wearables[0];
			}
			// Thanks, Beakers
			glove->SetGloveModelIndex(-1);
			apply_config_on_attributable_item(glove, glove_config, player_info.xuid_low);
		}
	}
	{
		auto weapons = local->m_hMyWeapons();
		for (int i = 0; weapons[i].IsValid(); i++)
		{
			C_BaseAttributableItem *weapon = (C_BaseAttributableItem*)Interfaces::EntityList->GetClientEntityFromHandle(weapons[i]);
			if (!weapon)
				continue;

			if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_ZEUSX27 
				|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_C4
				|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SMOKEGRENADE
				|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_FLASHBANG
				|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_HEGRENADE
				|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_DECOY
				|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_MOLOTOV
				|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_INC)
				continue;

			const auto active_conf = &Vars.weapon[GetWeaponSettingsSelectID_ForSkins(weapon)];

			apply_config_on_attributable_item(weapon, active_conf, player_info.xuid_low);
		}
	}
	const auto view_model_handle = local->m_hViewModel();
	if (!view_model_handle.IsValid())
		return;
	const auto view_model = static_cast<C_BaseViewModel*>(Interfaces::EntityList->GetClientEntityFromHandle(view_model_handle));
	if (!view_model)
		return;
	const auto view_model_weapon_handle = view_model->m_hWeapon();
	if (!view_model_weapon_handle.IsValid())
		return;
	const auto view_model_weapon = static_cast<C_BaseCombatWeapon*>(Interfaces::EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
	if (!view_model_weapon)
		return;
	if (k_weapon_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex()))
	{
		const auto override_model = k_weapon_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
		auto override_model_index = Interfaces::ModelInfo->GetModelIndex(override_model);
		view_model->m_nModelIndex() = override_model_index;
		auto world_model_handle = view_model_weapon->m_hWeaponWorldModel();
		if (!world_model_handle.IsValid())
			return;
		const auto world_model = static_cast<C_BaseWeaponWorldModel*>(Interfaces::EntityList->GetClientEntityFromHandle(world_model_handle));
		if (!world_model)
			return;
		world_model->m_nModelIndex() = override_model_index + 1;
	}
}

std::vector<paint_kit> k_skins;
std::vector<paint_kit> k_gloves;
class CCStrike15ItemSchema;
class CCStrike15ItemSystem;
template <typename Key, typename Value>
struct Node_t
{
	int previous_id; //0x0000
	int next_id; //0x0004
	void* _unknown_ptr; //0x0008
	int _unknown; //0x000C
	Key key; //0x0010
	Value value; //0x0014
};
template <typename Key, typename Value>
struct Head_t
{
	Node_t<Key, Value>* memory; //0x0000
	int allocation_count; //0x0004
	int grow_size; //0x0008
	int start_element; //0x000C
	int next_available; //0x0010
	int _unknown; //0x0014
	int last_element; //0x0018
}; 

struct String_t
{
	char* buffer; //0x0000
	int capacity; //0x0004
	int grow_size; //0x0008
	int length; //0x000C
};
struct CPaintKit
{
	int id; //0x0000
	String_t name; //0x0004
	String_t description; //0x0014
	String_t item_name; //0x0024
	String_t material_name; //0x0034
	String_t image_inventory; //0x0044
	char pad_0x0054[0x8C]; //0x0054
};
auto get_export(const char* module_name, const char* export_name) -> void*
{
	HMODULE mod;
	while (!((mod = GetModuleHandleA(module_name))))
		Sleep(100);
	return reinterpret_cast<void*>(GetProcAddress(mod, export_name));
}

auto CSkinChanger::Initialize() -> void
{
	const auto V_UCS2ToUTF8 = static_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(get_export("vstdlib.dll", "V_UCS2ToUTF8"));
	const auto sig_address = Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "E8 ? ? ? ? FF 76 0C 8D 48 04 E8");
	const auto item_system_offset = *reinterpret_cast<std::int32_t*>(sig_address + 1);
	const auto item_system_fn = reinterpret_cast<CCStrike15ItemSystem* (*)()>(sig_address + 5 + item_system_offset);
	const auto item_schema = reinterpret_cast<CCStrike15ItemSchema*>(std::uintptr_t(item_system_fn()) + sizeof(void*));
	{
		const auto get_paint_kit_definition_offset = *reinterpret_cast<std::int32_t*>(sig_address + 11 + 1);
		const auto get_paint_kit_definition_fn = reinterpret_cast<CPaintKit*(__thiscall*)(CCStrike15ItemSchema*, int)>(sig_address + 11 + 5 + get_paint_kit_definition_offset);
		const auto start_element_offset = *reinterpret_cast<std::intptr_t*>(std::uintptr_t(get_paint_kit_definition_fn) + 8 + 2);
		const auto head_offset = start_element_offset - 12;
		const auto map_head = reinterpret_cast<Head_t<int, CPaintKit*>*>(std::uintptr_t(item_schema) + head_offset);
		Interfaces::Localize->AddFile("resource/csgo_english.txt");
		for (auto i = 0; i <= map_head->last_element; ++i)
		{
			const auto paint_kit = map_head->memory[i].value;
			if (paint_kit->id == 9001)
				continue;
			const auto wide_name = Interfaces::Localize->Find(paint_kit->item_name.buffer + 1);
			char name[256];
			V_UCS2ToUTF8(wide_name, name, sizeof(name));
			if (paint_kit->id < 10000)
				k_skins.push_back({ paint_kit->id, name });
			else
				k_gloves.push_back({ paint_kit->id, name });
		}
		std::sort(k_skins.begin(), k_skins.end());
		std::sort(k_gloves.begin(), k_gloves.end());
	}
}
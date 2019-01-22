#pragma once
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../valve_sdk/csgostructs.hpp"

class Hitbox 
{
	public:
		bool GetHitbox(C_BasePlayer* pPlayer, int HitboxID)
		{
			if (!pPlayer) return false;

			matrix3x4_t matrix[128];

			*(int*)((int)pPlayer + 236) |= 8;

			if (!pPlayer->SetupBones(matrix, 128, 0x00000100, GetTickCount64()))
				return false;

			*(int*)((int)pPlayer + 236) &= ~8;

			const model_t* mod = pPlayer->GetModel();

			if (!mod) return false;

			studiohdr_t* hdr = Interfaces::ModelInfo->GetStudiomodel(mod);

			if (!hdr) return false;

			mstudiohitboxset_t* set = hdr->GetHitboxSet(0);

			if (!set) return false;

			mstudiobbox_t* hitbox = set->GetHitbox(HitboxID);

			if (!hitbox) return false;

			Vector points[] =
			{
				((hitbox->bbmin + hitbox->bbmax) * .5f),
				Vector(hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmin.z),
				Vector(hitbox->bbmin.x, hitbox->bbmax.y, hitbox->bbmin.z),
				Vector(hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmin.z),
				Vector(hitbox->bbmax.x, hitbox->bbmin.y, hitbox->bbmin.z),
				Vector(hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmax.z),
				Vector(hitbox->bbmin.x, hitbox->bbmax.y, hitbox->bbmax.z),
				Vector(hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmax.z),
				Vector(hitbox->bbmax.x, hitbox->bbmin.y, hitbox->bbmax.z) 
			};

			for (int i = 0; i < 9; i++) this->points[i] = points[i];

			return true;
		}

		Vector& GetCenter() 
		{
			return this->points[0];
		}

		Vector points[9];
};
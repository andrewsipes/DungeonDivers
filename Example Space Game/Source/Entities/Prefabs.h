// uses a nameless namespace to register and retreive prefabricated entities
#ifndef PREFABS_H
#define PREFABS_H

#include "../Precompiled.h"

// Dungeon Divers (avoid name collisions)
namespace DD
{
	bool RegisterPrefab(const char* prefabName, const flecs::entity inPrefab);
	bool RetreivePrefab(const char* prefabName, flecs::entity &outPrefab);
	bool UnregisterPrefab(const char* prefabName);
}

#endif
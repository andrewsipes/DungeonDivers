#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>

// handles everything
#include "Application.h"

// program entry point
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Application dungeonDivers;
	if (dungeonDivers.Init()) {
		if (dungeonDivers.Run()) {

			
			return dungeonDivers.Shutdown() ? 0 : 1;
		}
	}

	_CrtDumpMemoryLeaks();

	return 1;

}
// define the priorities of systems in the application
namespace ECS {

	enum InitPriority {
		preInit = 1,
		init = 2,
		postInit = 3,
	};

	enum RunPriority {
		preUpdate = 1,
		update = 2,
		postUpdate = 3,
		preRender = 4,
		render = 5,
		postRender = 6
	};

	enum ClosePriority {
		preClose = 1,
		close = 2,
		postClose = 3,
		exit = 4
	};

} // namespace ECS
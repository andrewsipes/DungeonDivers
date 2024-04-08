namespace ECS {

    // All components, tags, and systems are stored in a single registry
    entt::registry registry;
    
    // Priority guide:
    // 1 = Run First, 2 = Run Second, etc...

    // for systems, we use a priority queue
    // the int is the priority, and the function is the system
    typedef std::pair<int, std::function<bool(entt::registry&)>> SYSTEM;
    // sorts all systems by priority
    struct sort_systems {
        bool operator()(const SYSTEM& a, const SYSTEM& b) const {
            return a.first < b.first;
        }
    };
    // these systems are run once before any continuous systems and then removed
    // a return of false from a initialize system will force an abort()
    std::multiset<SYSTEM, sort_systems> initialize;
    // adds a system to the initialize list
    struct add_init_system {
		add_init_system(int priority, std::function<bool(entt::registry&)> system) {
			ECS::initialize.insert(std::make_pair(priority, system));
		}
	};
    // these systems are run once every frame and not removed
    // a return of false from a continuous system will exit the main loop
    std::multiset<SYSTEM, sort_systems> continuous;
    // adds a system to the continuous list
    struct add_continuous_system {
        add_continuous_system(int priority, std::function<bool(entt::registry&)> system) {
            ECS::continuous.insert(std::make_pair(priority, system));
        }
    };
    // a return of false from a shutdown system will force an abort()
    // these systems are run once after the main loop before main returns
    std::multiset<SYSTEM, sort_systems> shutdown;
    // adds a system to the shutdown list
    struct add_shutdown_system {
		add_shutdown_system(int priority, std::function<bool(entt::registry&)> system) {
			ECS::shutdown.insert(std::make_pair(priority, system));
		}
	};

    // Run the ECS system
    inline bool IterateSystems() {
        // run all initialization systems
        for (auto& init : ECS::initialize) {
            if (std::invoke(init.second, ECS::registry) == false) {
                abort();
            }
        }
        // remove all initialization systems
        ECS::initialize.clear(); 

        // run all continuous systems
        for (auto& run : ECS::continuous) {
            if (std::invoke(run.second, ECS::registry) == false) {
                return false;
            }
        }
        // continue running while there is something to do
        return !ECS::continuous.empty(); 
    }
    // Shutdown the ECS system
    inline bool ShutdownSystems() {
        // run all shutdown systems
        for (auto& stop : ECS::shutdown) {
            if (std::invoke(stop.second, ECS::registry) == false) {
                abort();
            }
        }
        return true; // shutdown complete
    }

} // namespace ECS
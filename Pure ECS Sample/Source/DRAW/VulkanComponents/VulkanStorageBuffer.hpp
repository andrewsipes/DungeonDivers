namespace DRAW {
	
	//*** TAGS ***//
	
	//*** COMPONENTS ***//
	struct GPU_INSTANCE {
		GW::MATH::GMATRIXF world;
		H2B::ATTRIBUTES material;
	};
	template<typename T>
	struct VulkanStorageBuffer {
		unsigned long long element_count = 1;
		std::vector<VkBuffer> buffer;
		std::vector<VkDeviceMemory> memory;
	};
	
	//*** SYSTEMS ***//
	// prototypes for the systems (required since they invoke one another in this case) 
	template<typename T>
	inline void Construct_VulkanStorageBuffer(entt::registry& registry, entt::entity entity);
	template<typename T>
	inline void Update_VulkanStorageBuffer(entt::registry& registry, entt::entity entity);
	template<typename T>
	inline void Destroy_VulkanStorageBuffer(entt::registry& registry, entt::entity entity);

	// implementations of the systems
	template<typename T>
	inline void Construct_VulkanStorageBuffer(entt::registry& registry, entt::entity entity) {
		auto& storage = registry.get<VulkanStorageBuffer<T>>(entity);
		if (registry.all_of<GW::GRAPHICS::GVulkanSurface>(entity)) {
			auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
			// Grab the device & physical device so we can allocate some stuff
			VkDevice device = nullptr;
			VkPhysicalDevice physicalDevice = nullptr;
			vkSurface.GetDevice((void**)&device);
			vkSurface.GetPhysicalDevice((void**)&physicalDevice);
			// allocate all required buffers based on frame count
			unsigned max_frames = 0;
			// to avoid per-frame resource sharing issues we give each "in-flight" frame its own buffer
			vkSurface.GetSwapchainImageCount(max_frames);
			storage.buffer.resize(max_frames);
			storage.memory.resize(max_frames);
			for (int i = 0; i < max_frames; ++i) {
				// allocate each buffers size based on the count of elements
				GvkHelper::create_buffer(physicalDevice, device, sizeof(T) * storage.element_count,
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storage.buffer[i], &storage.memory[i]);
			}
		}
	}
	template<typename T>
	inline void Update_VulkanStorageBuffer(entt::registry& registry, entt::entity entity) {
		auto& storage = registry.get<VulkanStorageBuffer<T>>(entity);
		// update contents of the storage buffer based on active frame
		if (registry.all_of<GW::GRAPHICS::GVulkanSurface, std::vector<T>>(entity)) {
			auto& transfer = registry.get<std::vector<T>>(entity);
			auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
			// if the gpu buffers are not big enough, lets destroy them and recreate them
			if (storage.element_count < transfer.size()) {
				Destroy_VulkanVertexBuffer(registry, entity);
				storage.element_count = transfer.size(); // get enough space for the new data
				Construct_VulkanStorageBuffer<T>(registry, entity);
				return Update_VulkanStorageBuffer<T>(registry, entity);
			}
			// Grab the device & physical device so we can allocate some stuff
			VkDevice device = nullptr;
			vkSurface.GetDevice((void**)&device);
			// Write data to this frame's buffer
			unsigned int currentBuffer = 0;
			vkSurface.GetSwapchainCurrentImage(currentBuffer);
			GvkHelper::write_to_buffer(device, storage.memory[currentBuffer],
				transfer.data(), sizeof(T) * transfer.size());
		}
	}
	template<typename T>
	inline void Destroy_VulkanStorageBuffer(entt::registry& registry, entt::entity entity) {
		auto& storage = registry.get<VulkanStorageBuffer<T>>(entity);
		if (registry.all_of<GW::GRAPHICS::GVulkanSurface>(entity)) {
			VkDevice device = nullptr;
			auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
			vkSurface.GetDevice((void**)&device);
			// wait till everything has completed
			vkDeviceWaitIdle(device);
			// free the storage buffers and its handles
			for (int i = 0; i < storage.buffer.size(); ++i) {
				vkDestroyBuffer(device, storage.buffer[i], nullptr);
				vkFreeMemory(device, storage.memory[i], nullptr);
			}
			storage.buffer.clear();
			storage.memory.clear();
		}
	}
	
	// MACRO to connect the EnTT VulkanStorageBuffer<GPU_INSTANCE> Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<VulkanStorageBuffer<GPU_INSTANCE>>().connect<Construct_VulkanStorageBuffer<GPU_INSTANCE>>();
		registry.on_update<VulkanStorageBuffer<GPU_INSTANCE>>().connect<Update_VulkanStorageBuffer<GPU_INSTANCE>>();
		registry.on_destroy<VulkanStorageBuffer<GPU_INSTANCE>>().connect<Destroy_VulkanStorageBuffer<GPU_INSTANCE>>();
	}
	
} // namespace DRAW
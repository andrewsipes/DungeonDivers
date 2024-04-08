namespace DRAW {
	
	//*** TAGS ***//

	//*** COMPONENTS ***//
	struct VulkanIndexBuffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
	};

	//*** SYSTEMS ***//
	// prototype.used below in function Update_VulkanVertexBuffer
	inline void Destroy_VulkanIndexBuffer(entt::registry & registry, entt::entity entity);
	
	// Boilerplate for logic that runs when adding a new component to the registry
	inline void Construct_VulkanIndexBuffer(entt::registry& registry, entt::entity entity) {
		//auto& gpuBuffer = registry.get<VulkanIndexBuffer>(entity);
	}

	// Boilerplate for logic that runs when updating a component via "patch"
	inline void Update_VulkanIndexBuffer(entt::registry& registry, entt::entity entity) {
		auto& gpuBuffer = registry.get<VulkanIndexBuffer>(entity);
		// upload the buffer to the GPU
		if (registry.all_of<GW::GRAPHICS::GVulkanSurface, std::vector<H2B::VERTEX>>(entity)) {
			// if there is already a gpu buffer attached, lets delete it
			if (gpuBuffer.buffer != VK_NULL_HANDLE)
				Destroy_VulkanIndexBuffer(registry, entity);
			// if there is a cpu buffer attached, lets upload it to the GPU then delete it
			auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
			auto& cpuBuffer = registry.get<std::vector<unsigned int>>(entity);
			// Grab the device & physical device so we can allocate some stuff
			VkDevice device = nullptr;
			VkPhysicalDevice physicalDevice = nullptr;
			vkSurface.GetDevice((void**)&device);
			vkSurface.GetPhysicalDevice((void**)&physicalDevice);
			// Transfer triangle data to the vertex buffer. (staging would be preferred here)
			GvkHelper::create_buffer(physicalDevice, device, sizeof(unsigned int) * cpuBuffer.size(),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &gpuBuffer.buffer, &gpuBuffer.memory);
			GvkHelper::write_to_buffer(device, gpuBuffer.memory,
				cpuBuffer.data(), sizeof(unsigned int) * cpuBuffer.size());
			// remove the CPU buffer
			registry.remove<std::vector<unsigned int>>(entity);
		}
	}

	// Boilerplate for logic that runs when removing a component from the registry
	inline void Destroy_VulkanIndexBuffer(entt::registry& registry, entt::entity entity) {
		// check if the buffer is allocated, if so, release it
		if (registry.all_of<VulkanIndexBuffer, GW::GRAPHICS::GVulkanSurface>(entity)) {

			auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
			auto& gpuBuffer = registry.get<VulkanIndexBuffer>(entity);
			// Grab the device & physical device so we can allocate some stuff
			VkDevice device = nullptr;
			vkSurface.GetDevice((void**)&device);
			vkDeviceWaitIdle(device);
			// Release allocated buffers, shaders & pipeline
			vkDestroyBuffer(device, gpuBuffer.buffer, nullptr);
			vkFreeMemory(device, gpuBuffer.memory, nullptr);
		}
	}
	
	// MACRO to connect the EnTT Component Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<VulkanIndexBuffer>().connect<Construct_VulkanIndexBuffer>();
		registry.on_update<VulkanIndexBuffer>().connect<Update_VulkanIndexBuffer>();
		registry.on_destroy<VulkanIndexBuffer>().connect<Destroy_VulkanIndexBuffer>();
		// We could do this in main.cpp, but this is more organized
	}
	

} // namespace DRAW
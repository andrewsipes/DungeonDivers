namespace DRAW {
	
	//*** TAGS ***//
	
	//*** COMPONENTS ***//
	struct VulkanVertexBuffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
	};
	
	//*** SYSTEMS ***//

	// prototype. used below in function Update_VulkanVertexBuffer
	inline void Destroy_VulkanVertexBuffer(entt::registry& registry, entt::entity entity);
	
	inline void Construct_VulkanVertexBuffer(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<VulkanVertexBuffer>(entity);
	}
	
	inline void Update_VulkanVertexBuffer(entt::registry& registry, entt::entity entity) {
		auto& gpuBuffer = registry.get<VulkanVertexBuffer>(entity);
		// upload the buffer to the GPU
		if (registry.all_of<GW::GRAPHICS::GVulkanSurface, std::vector<H2B::VERTEX>>(entity)) {
			// if there is already a gpu buffer attached, lets delete it
			if (gpuBuffer.buffer != VK_NULL_HANDLE)
				Destroy_VulkanVertexBuffer(registry, entity);
			// if there is a cpu buffer attached, lets upload it to the GPU then delete it
			auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
			auto& cpuBuffer = registry.get<std::vector<H2B::VERTEX>>(entity);
			// Grab the device & physical device so we can allocate some stuff
			VkDevice device = nullptr;
			VkPhysicalDevice physicalDevice = nullptr;
			vkSurface.GetDevice((void**)&device);
			vkSurface.GetPhysicalDevice((void**)&physicalDevice);
			// Transfer triangle data to the vertex buffer. (staging would be preferred here)
			GvkHelper::create_buffer(physicalDevice, device, sizeof(H2B::VERTEX) * cpuBuffer.size(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &gpuBuffer.buffer, &gpuBuffer.memory);
			GvkHelper::write_to_buffer(device, gpuBuffer.memory, 
				cpuBuffer.data(), sizeof(H2B::VERTEX) * cpuBuffer.size());
			// remove the CPU buffer
			registry.remove<std::vector<H2B::VERTEX>>(entity);
		}
	}

	// implementation. used above in function Update_VulkanVertexBuffer
	inline void Destroy_VulkanVertexBuffer(entt::registry& registry, entt::entity entity) {
		// check if the buffer is allocated, if so, release it
		if (registry.all_of<VulkanVertexBuffer, GW::GRAPHICS::GVulkanSurface>(entity)) {
			
			auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
			auto& gpuBuffer = registry.get<VulkanVertexBuffer>(entity);
			// Grab the device & physical device so we can allocate some stuff
			VkDevice device = nullptr;
			vkSurface.GetDevice((void**)&device);
			vkDeviceWaitIdle(device);
			// Release allocated buffers, shaders & pipeline
			vkDestroyBuffer(device, gpuBuffer.buffer, nullptr);
			vkFreeMemory(device, gpuBuffer.memory, nullptr);
		}
		
	}
	
	// MACRO to connect the EnTT VulkanVertexBuffer Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<VulkanVertexBuffer>().connect<Construct_VulkanVertexBuffer>();
		registry.on_update<VulkanVertexBuffer>().connect<Update_VulkanVertexBuffer>();
		registry.on_destroy<VulkanVertexBuffer>().connect<Destroy_VulkanVertexBuffer>();
		// We could do this in main.cpp, but this is more organized
	}
	

} // namespace DRAW
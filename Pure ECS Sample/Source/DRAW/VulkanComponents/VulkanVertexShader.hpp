// component dependencies
#include "../Utility/FileIntoString.h"

#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MT platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") 
#endif

namespace DRAW {
	
	//*** TAGS ***//

	//*** COMPONENTS ***//
	struct VulkanVertexShader {
		std::string filePath;
		VkShaderModule vertexShader = nullptr;
	};

	//*** SYSTEMS ***//
	
	// Boilerplate for logic that runs when adding a new component to the registry
	inline void Construct_VulkanVertexShader(entt::registry& registry, entt::entity entity) {
		auto& vulkanVertShader = registry.get<VulkanVertexShader>(entity);

		std::string vertexShaderSource = ReadFileIntoString(vulkanVertShader.filePath.c_str());

		// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(options, false);
#ifndef NDEBUG
		shaderc_compile_options_set_generate_debug_info(options);
#endif
		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, vertexShaderSource.c_str(), vertexShaderSource.length(),
			shaderc_vertex_shader, "main.vert", "main", options);

		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
		{
			std::cout << "Vertex Shader Errors : \n" << shaderc_result_get_error_message(result) << std::endl;
			abort();
			return;
		}
		auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
		VkDevice device = nullptr;
		vkSurface.GetDevice((void**)&device);

		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &vulkanVertShader.vertexShader);

		shaderc_result_release(result); // done

		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);

		vulkanVertShader.filePath.clear();
	}

	// Boilerplate for logic that runs when updating a component via "patch"
	inline void Update_VulkanVertexShader(entt::registry& registry, entt::entity entity) {
		auto& vulkanVertShader = registry.get<VulkanVertexShader>(entity);
	}

	// Boilerplate for logic that runs when removing a component from the registry
	inline void Destroy_VulkanVertexShader(entt::registry& registry, entt::entity entity) {
		auto& vulkanVertShader = registry.get<VulkanVertexShader>(entity);
		auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
		VkDevice device = nullptr;
		vkSurface.GetDevice((void**)&device);

		vkDestroyShaderModule(device, vulkanVertShader.vertexShader, nullptr);
	}
	
	// MACRO to connect the EnTT Component Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<VulkanVertexShader>().connect<Construct_VulkanVertexShader>();
		registry.on_update<VulkanVertexShader>().connect<Update_VulkanVertexShader>();
		registry.on_destroy<VulkanVertexShader>().connect<Destroy_VulkanVertexShader>();
		// We could do this in main.cpp, but this is more organized
	}
	

} // namespace DRAW
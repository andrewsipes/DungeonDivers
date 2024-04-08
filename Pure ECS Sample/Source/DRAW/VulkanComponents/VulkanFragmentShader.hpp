// component dependencies
#include "../Utility/FileIntoString.h"

#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MT platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") 
#endif

namespace DRAW {
	
	//*** TAGS ***//

	//*** COMPONENTS ***//
	struct VulkanFragmentShader {
		std::string filePath;
		VkShaderModule fragmentShader = nullptr;
	};

	//*** SYSTEMS ***//
	
	// Boilerplate for logic that runs when adding a new component to the registry
	inline void Construct_VulkanFragmentShader(entt::registry& registry, entt::entity entity) {
		auto& vulkanFragmentShader = registry.get<VulkanFragmentShader>(entity);

		std::string fragmentShaderSource = ReadFileIntoString(vulkanFragmentShader.filePath.c_str());

		// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(options, false);
#ifndef NDEBUG
		shaderc_compile_options_set_generate_debug_info(options);
#endif
		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, fragmentShaderSource.c_str(), fragmentShaderSource.length(),
			shaderc_fragment_shader, "main.frag", "main", options);

		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors? 
		{
			std::cout << "Fragment Shader Errors : \n" << shaderc_result_get_error_message(result) << std::endl;
			abort();
			return;
		}
		auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
		VkDevice device = nullptr;
		vkSurface.GetDevice((void**)&device);

		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &vulkanFragmentShader.fragmentShader);

		shaderc_result_release(result); // done

		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);

		vulkanFragmentShader.filePath.clear();
	}

	// Boilerplate for logic that runs when updating a component via "patch"
	inline void Update_VulkanFragmentShader(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<VulkanFragmentShader>(entity);
	}

	// Boilerplate for logic that runs when removing a component from the registry
	inline void Destroy_VulkanFragmentShader(entt::registry& registry, entt::entity entity) {
		auto& vulkanFragmentShader = registry.get<VulkanFragmentShader>(entity);
		auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
		VkDevice device = nullptr;
		vkSurface.GetDevice((void**)&device);

		vkDestroyShaderModule(device, vulkanFragmentShader.fragmentShader, nullptr);
	}
	
	// MACRO to connect the EnTT Component Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<VulkanFragmentShader>().connect<Construct_VulkanFragmentShader>();
		registry.on_update<VulkanFragmentShader>().connect<Update_VulkanFragmentShader>();
		registry.on_destroy<VulkanFragmentShader>().connect<Destroy_VulkanFragmentShader>();
		// We could do this in main.cpp, but this is more organized
	}
	

} // namespace DRAW
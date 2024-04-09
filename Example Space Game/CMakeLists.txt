cmake_minimum_required(VERSION 3.16)

project(UntitledGame)

set(PRE_COMPILED
    ./Source/precompiled.h 
)

set(VERTEX_SHADERS 
	# add vertex shader (.glsl) files here
	./Shaders/VertexShader.glsl
)

set(PIXEL_SHADERS 
	# add pixel shader (.glsl) files here
	./Shaders/FragmentShader.glsl
)



# Add any new C/C++ source code here
file(
GLOB_RECURSE SOURCE_CODE CONFIGURE_DEPENDS
	./Source/*.h
	./Source/*.cpp
    #./Shaders/*.hlsl
	./gateware-main/Gateware.h
	./flecs-3.1.4/flecs.h
    ./flecs-3.1.4/flecs.c
	./inifile-cpp-master/include/inicpp.h
	./defaults.ini # get some syntax highlighting
)

# The FLECS library core is written in C. If it tries to use the C++ pre-compiled header it will not compile.
set_source_files_properties(./flecs-3.1.4/flecs.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
# Example: You can use this to remove files so later you can add them individually if needed.
#list(FILTER SOURCE_FILES EXCLUDE REGEX ".*main.*.cpp$")

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if(WIN32)
	# by default CMake selects "ALL_BUILD" as the startup project 
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
		 PROPERTY VS_STARTUP_PROJECT UntitledGame)

	#VULKAN ONLY KEEP COMMENTED
	#source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCE_FILES})
	#target_include_directories(UntitledGame PUBLIC $ENV{VULKAN_SDK}/Include/)
	#target_link_directories(UntitledGame PUBLIC $ENV{VULKAN_SDK}/Lib/)
	#target_compile_options(UntitledGame PRIVATE "/MD")

	#this will tell Visual Studio to put the shaders under their own group in the solution explorer 
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VERTEX_SHADERS})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PIXEL_SHADERS})
endif()

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if (UNIX AND NOT APPLE)
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lX11 -lGL")
     find_package(X11)
     find_package(OpenGL)
     include_directories(${X11_INCLUDE_DIR})
     link_libraries(${X11_LIBRARIES})
     include_directories(${OPENGL_INCLUDE_DIRS})
     link_libraries(${OPENGL_LIBRARIES})
endif()

add_executable (UntitledGame
	${SOURCE_CODE}
	${VERTEX_SHADERS}
	${PIXEL_SHADERS})

target_compile_features(UntitledGame PUBLIC cxx_std_17)

# adding gateware.h and other librairies as a precompiled headers to reduce compile times
target_precompile_headers(UntitledGame PRIVATE ${PRE_COMPILED})

# Disable all shaders from compiling since they may have OpenGL specifics
file(
	GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
    ./Shaders/*.glsl
)



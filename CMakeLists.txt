# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies 
add_subdirectory (engine/external/external/)

add_definitions(-O3) # compiler flag
add_definitions(-g) # compiler flag

include_directories(
	engine/external/
	engine/external/external/glfw-3.1.2/include/
	engine/external/external/glm-0.9.7.1/
	engine/external/external/glew-1.13.0/include/
	engine/
	game/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

add_executable(Game
	engine/external/common/shader.cpp
	engine/external/common/shader.hpp
	engine/external/common/objloader.cpp
	engine/external/common/objloader.hpp
	engine/external/common/vboindexer.cpp
	engine/external/common/vboindexer.hpp

	engine/core/resourceLoader/resourceLoader.h
	engine/core/resourceLoader/resourceLoader.cpp

	engine/render/texture/texture.h
	engine/render/texture/texture.cpp

	
	engine/utils/utils.h
	engine/utils/utils.cpp



	engine/core/component/component.h
	engine/core/component/component.cpp

	engine/core/gameObject/gameObject.h
	engine/core/gameObject/gameObject.cpp

	engine/core/scene/scene.h
	engine/core/scene/scene.cpp

	engine/core/game/game.h
	engine/core/game/game.cpp

	engine/core/inputs/input.h
	engine/core/inputs/input.cpp


	# base components definitions
	engine/core/core_components/components.h
	engine/core/core_components/components.cpp

	engine/render/components/components.h
	engine/render/components/components.cpp

	engine/render/mesh/mesh.h
	engine/render/mesh/mesh.cpp # order is important!!!
	engine/render/mesh/mesh_gen.cpp

	engine/external/common/tangentspace.cpp
	engine/external/common/tangentspace.hpp

	engine/external/stb_image.cpp

	# rendering

	engine/render/renderingServer/renderingServer.h
	engine/render/renderingServer/renderingServer.cpp

	engine/render/material/material.h
	engine/render/material/material.cpp

	# physics

	engine/physics/collider/collider.h
	engine/physics/components/components.h
	engine/physics/components/components.cpp

	engine/physics/physics_server/physics_server.h
	engine/physics/physics_server/physics_server.cpp

	# game
    	game/game.cpp
	
	game/src/voxel/voxel.h
	game/src/voxel/voxel.cpp
	game/src/voxel/mapGeneration.cpp
	game/src/controller/playerController.h
)

target_link_libraries(Game
	${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(Game PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")




SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )


if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
add_custom_command(
   TARGET Game POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/Game${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/"
)
elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )


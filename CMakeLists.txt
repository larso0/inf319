cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")
project(inf319)

set(MIDDLEWARE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Include)

set(ENGINE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Include)
set(ENGINE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source)
set(ENGINE_SRC_FILES
	${ENGINE_SRC}/Input.cpp
	${ENGINE_SRC}/Math.cpp
	${ENGINE_SRC}/MeshGeneration.cpp
	${ENGINE_SRC}/Node.cpp
	${ENGINE_SRC}/Vertex.cpp
)

set(SANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Sandbox/Source)
set(SANDBOX_SRC_FILES
	${SANDBOX_SRC}/glad.c
	${SANDBOX_SRC}/GLContext.cpp
	${SANDBOX_SRC}/GLPerMesh.cpp
	${SANDBOX_SRC}/GLRenderer.cpp
	${SANDBOX_SRC}/GLWindow.cpp
	${SANDBOX_SRC}/Main.cpp
)

set(VULKANSANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/VulkanSandbox/Source)
set(VULKANSANDBOX_SRC_FILES
	${VULKANSANDBOX_SRC}/Main.cpp
	${VULKANSANDBOX_SRC}/VulkanBuffer.cpp
	${VULKANSANDBOX_SRC}/VulkanContext.cpp
	${VULKANSANDBOX_SRC}/VulkanDevice.cpp
	${VULKANSANDBOX_SRC}/VulkanPerMesh.cpp
	${VULKANSANDBOX_SRC}/VulkanRenderer.cpp
	${VULKANSANDBOX_SRC}/VulkanShaderProgram.cpp
	${VULKANSANDBOX_SRC}/VulkanWindow.cpp
)
set(VULKANSANDBOX_SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/VulkanSandbox/Shaders)
set(VULKANSANDBOX_SHADER_SRC_FILES
	${VULKANSANDBOX_SHADER_SRC}/Simple.frag
	${VULKANSANDBOX_SHADER_SRC}/Simple.vert
)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

find_package(GLFW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(
	${MIDDLEWARE_INCLUDE}
	${ENGINE_INCLUDE}
	${GLFW_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIRS}
	${Vulkan_INCLUDE_DIRS}
)

add_library(Engine STATIC ${ENGINE_SRC_FILES})

add_executable(Sandbox ${SANDBOX_SRC_FILES})
add_dependencies(Sandbox Engine)
target_link_libraries(Sandbox ${GLFW_LIBRARY} ${OPENGL_LIBRARIES} Engine)

add_executable(VulkanSandbox ${VULKANSANDBOX_SRC_FILES})
add_dependencies(VulkanSandbox Engine)
target_link_libraries(VulkanSandbox ${GLFW_LIBRARY} ${Vulkan_LIBRARY} Engine)

foreach(SHADER_SRC ${VULKANSANDBOX_SHADER_SRC_FILES})
	get_filename_component(FILE_NAME ${SHADER_SRC} NAME)
	set(SPIRV "${PROJECT_BINARY_DIR}/Shaders/${FILE_NAME}.spv")
	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/Shaders/"
		COMMAND glslangValidator -V ${SHADER_SRC} -o ${SPIRV}
		DEPENDS ${SHADER_SRC})
	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(SHADER_SRC)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(VulkanSandbox Shaders)

add_custom_command(TARGET VulkanSandbox POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:VulkanSandbox>/Shaders/"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_BINARY_DIR}/Shaders"
		"$<TARGET_FILE_DIR:VulkanSandbox>/Shaders"
)
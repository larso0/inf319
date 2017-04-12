cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")
project(inf319)

set(MIDDLEWARE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Include)

set(ENGINE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Include)
set(ENGINE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source)
set(ENGINE_SRC_FILES
	${ENGINE_INCLUDE}/Engine/Camera.h
	${ENGINE_INCLUDE}/Engine/Context.h
	${ENGINE_INCLUDE}/Engine/Entity.h
	${ENGINE_INCLUDE}/Engine/EventHandler.h
	${ENGINE_INCLUDE}/Engine/IndexedMesh.h
	${ENGINE_INCLUDE}/Engine/Input.h
	${ENGINE_INCLUDE}/Engine/KeyEventHandler.h
	${ENGINE_INCLUDE}/Engine/LightSource.h
	${ENGINE_INCLUDE}/Engine/Material.h
	${ENGINE_INCLUDE}/Engine/Math.h
	${ENGINE_INCLUDE}/Engine/Mesh.h
	${ENGINE_INCLUDE}/Engine/MeshGeneration.h
	${ENGINE_INCLUDE}/Engine/MouseEventHandler.h
	${ENGINE_INCLUDE}/Engine/Node.h
	${ENGINE_INCLUDE}/Engine/Renderer.h
	${ENGINE_INCLUDE}/Engine/Texture.h
	${ENGINE_INCLUDE}/Engine/Vertex.h
	${ENGINE_INCLUDE}/Engine/Window.h
	${ENGINE_INCLUDE}/Engine/WindowEventHandler.h
	${ENGINE_SRC}/Input.cpp
	${ENGINE_SRC}/Math.cpp
	${ENGINE_SRC}/MeshGeneration.cpp
	${ENGINE_SRC}/Node.cpp
	${ENGINE_SRC}/Texture.cpp
	${ENGINE_SRC}/Vertex.cpp
)

set(SANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Sandbox/Source)
set(SANDBOX_SRC_FILES
	${SANDBOX_SRC}/glad.c
	${SANDBOX_SRC}/GLContext.cpp
	${SANDBOX_SRC}/GLContext.h
	${SANDBOX_SRC}/GLPerMesh.cpp
	${SANDBOX_SRC}/GLPerMesh.h
	${SANDBOX_SRC}/GLRenderer.cpp
	${SANDBOX_SRC}/GLRenderer.h
	${SANDBOX_SRC}/GLWindow.cpp
	${SANDBOX_SRC}/GLWindow.h
	${SANDBOX_SRC}/Main.cpp
)

set(VULKANSANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/VulkanSandbox/Source)
set(VULKANSANDBOX_SRC_FILES
	${VULKANSANDBOX_SRC}/Main.cpp
	${VULKANSANDBOX_SRC}/VulkanBuffer.cpp
	${VULKANSANDBOX_SRC}/VulkanBuffer.h
	${VULKANSANDBOX_SRC}/VulkanContext.cpp
	${VULKANSANDBOX_SRC}/VulkanContext.h
	${VULKANSANDBOX_SRC}/VulkanDevice.cpp
	${VULKANSANDBOX_SRC}/VulkanDevice.h
	${VULKANSANDBOX_SRC}/VulkanImage.cpp
	${VULKANSANDBOX_SRC}/VulkanImage.h
	${VULKANSANDBOX_SRC}/VulkanIncludes.h
	${VULKANSANDBOX_SRC}/VulkanPerMesh.cpp
	${VULKANSANDBOX_SRC}/VulkanPerMesh.h
	${VULKANSANDBOX_SRC}/VulkanRenderer.cpp
	${VULKANSANDBOX_SRC}/VulkanRenderer.h
	${VULKANSANDBOX_SRC}/VulkanShaderProgram.cpp
	${VULKANSANDBOX_SRC}/VulkanShaderProgram.h
	${VULKANSANDBOX_SRC}/VulkanSwapchain.cpp
	${VULKANSANDBOX_SRC}/VulkanSwapchain.h
	${VULKANSANDBOX_SRC}/VulkanUtil.cpp
	${VULKANSANDBOX_SRC}/VulkanUtil.h
	${VULKANSANDBOX_SRC}/VulkanWindow.cpp
	${VULKANSANDBOX_SRC}/VulkanWindow.h
)
set(VULKANSANDBOX_SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/VulkanSandbox/Shaders)
set(VULKANSANDBOX_SHADER_SRC_FILES
	${VULKANSANDBOX_SHADER_SRC}/Simple.frag
	${VULKANSANDBOX_SHADER_SRC}/Simple.vert
)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
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

if(CMAKE_COMPILER_IS_GNUCXX)
	target_link_libraries(Sandbox dl)
endif()

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

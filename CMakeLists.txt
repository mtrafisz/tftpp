cmake_minimum_required (VERSION 3.5)

# make sure the submodules are initialized:
execute_process (COMMAND git submodule update --init --recursive --depth 1
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

project (tftpp)

# compile submodules
add_subdirectory (third-party/glfw)
add_subdirectory (third-party/tftp.cpp)
add_subdirectory (third-party/nfd) 

# set imgui backend to use glfw and opengl3
set (IMGUI_BACKENDS
     third-party/imgui/backends/imgui_impl_glfw.cpp
     third-party/imgui/backends/imgui_impl_opengl3.cpp)

# add our source files
file (GLOB SOURCES source/*.cpp)
add_executable (${PROJECT_NAME} ${SOURCES})

# include imgui and glfw headers
target_include_directories (${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/tftp.cpp/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/nfd/src/include
)

# add imgui source files
file (GLOB IMGUI_SOURCES third-party/imgui/*.cpp)
target_sources (${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES} ${IMGUI_BACKENDS})

# link with submodules
target_link_libraries (${PROJECT_NAME} PRIVATE glfw opengl32 tftpc nfd)

# add platform specific libraries:
if (WIN32)
    target_link_libraries (${PROJECT_NAME} PRIVATE ws2_32)
endif()

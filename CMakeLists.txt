cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")
project(bp)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif ()

find_package(GLFW REQUIRED)
find_package(Vulkan REQUIRED)

set(BP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BP_SRC_FILES
    ${BP_INCLUDE_DIR}/bp/bp.h
    ${BP_INCLUDE_DIR}/bp/device.h
    ${BP_INCLUDE_DIR}/bp/ed.h
    ${BP_INCLUDE_DIR}/bp/vk_includes.h
    ${BP_INCLUDE_DIR}/bp/window.h
    ${BP_INCLUDE_DIR}/bp/swapchain.h
    ${BP_INCLUDE_DIR}/bp/buffer.h
    ${BP_INCLUDE_DIR}/bp/image.h
    ${BP_SRC_DIR}/device.cpp
    ${BP_SRC_DIR}/init.cpp
    ${BP_SRC_DIR}/window.cpp
    ${BP_SRC_DIR}/event_handling.cpp
    ${BP_SRC_DIR}/swapchain.cpp
    ${BP_SRC_DIR}/buffer.cpp
    ${BP_SRC_DIR}/image.cpp)

add_library(bp STATIC ${BP_SRC_FILES})
include_directories(
	${BP_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${Vulkan_INCLUDE_DIRS}
)

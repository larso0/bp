cmake_minimum_required(VERSION 3.3)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")
project(bp)

set(CMAKE_CXX_STANDARD 11)
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif ()

if (NOT BP_MODULES)
	set(BP_MODULES Scene;View;Qt)
endif()

set(BP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bp/include)
set(BP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bp/src)
file(GLOB_RECURSE BP_SRC_FILES "${BP_SRC_DIR}/*.cpp" "src/*.c")
file(GLOB_RECURSE BP_INCLUDE_FILES "${BP_INCLUDE_DIR}/*.h" "${BP_INCLUDE_DIR}/*.hpp")
set(BP_SRC ${BP_SRC_FILES} ${BP_INCLUDE_FILES})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS} ${BP_INCLUDE_DIR})
add_library(bp STATIC ${BP_SRC})

if (Scene IN_LIST BP_MODULES)
	set(BP_SCENE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bpScene/include)
	set(BP_SCENE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bpScene/src)
	file(GLOB_RECURSE BP_SCENE_SRC_FILES "${BP_SCENE_SRC_DIR}/*.cpp" "src/*.c")
	file(GLOB_RECURSE BP_SCENE_INCLUDE_FILES "${BP_SCENE_INCLUDE_DIR}/*.h" "${BP_SCENE_INCLUDE_DIR}/*.hpp")
	set(BP_SCENE_SRC ${BP_SCENE_SRC_FILES} ${BP_SCENE_INCLUDE_FILES})

	find_package(GLM REQUIRED)
	include_directories(${GLM_INCLUDE_DIRS} ${BP_SCENE_INCLUDE_DIR})
	add_library(bpScene STATIC ${BP_SCENE_SRC})
endif()
if (View IN_LIST BP_MODULES)
	set(BP_VIEW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bpView/include)
	set(BP_VIEW_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bpView/src)
	file(GLOB_RECURSE BP_VIEW_SRC_FILES "${BP_VIEW_SRC_DIR}/*.cpp" "src/*.c")
	file(GLOB_RECURSE BP_VIEW_INCLUDE_FILES "${BP_VIEW_INCLUDE_DIR}/*.h" "${BP_VIEW_INCLUDE_DIR}/*.hpp")
	set(BP_VIEW_SRC ${BP_VIEW_SRC_FILES} ${BP_VIEW_INCLUDE_FILES})

	find_package(GLFW REQUIRED)
	include_directories(${GLFW_INCLUDE_DIR} ${BP_VIEW_INCLUDE_DIR})
	add_library(bpView STATIC ${BP_VIEW_SRC})
endif()
if (Qt IN_LIST BP_MODULES)
	set(BP_QT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bpQt/include)
	set(BP_QT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bpQt/src)
	file(GLOB_RECURSE BP_QT_SRC_FILES "${BP_QT_SRC_DIR}/*.cpp" "src/*.c")
	file(GLOB_RECURSE BP_QT_INCLUDE_FILES "${BP_QT_INCLUDE_DIR}/*.h" "${BP_QT_INCLUDE_DIR}/*.hpp")
	set(BP_QT_SRC ${BP_QT_SRC_FILES} ${BP_QT_INCLUDE_FILES})

	set(CMAKE_AUTOMOC ON)
	find_package(Qt5Core REQUIRED)
	find_package(Qt5Gui REQUIRED)
	find_package(Qt5Widgets REQUIRED)

	if (Qt5Core_VERSION VERSION_LESS 5.10.0 OR
	    Qt5Gui_VERSION VERSION_LESS 5.10.0 OR
	    Qt5Widgets_VERSION VERSION_LESS 5.10.0)
		message(FATAL_ERROR "Minimum supported Qt version is 5.10.0")
	endif()

	if (Qt5_POSITION_INDEPENDENT_CODE)
		set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	endif()

	include_directories(${Qt5Core_INCLUDE_DIRS}
			    ${Qt5Gui_INCLUDE_DIRS}
			    ${Qt5Widgets_INCLUDE_DIRS}
			    ${BP_QT_INCLUDE_DIR})
	add_library(bpQt STATIC ${BP_QT_SRC})
endif()
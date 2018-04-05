cmake_minimum_required(VERSION 3.3)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
set(CMAKE_DEBUG_POSTFIX d)
project(bp)

set(CMAKE_CXX_STANDARD 11)
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

if (NOT BP_MODULES)
	set(BP_MODULES Multi;Qt;Scene;View)
endif()

set(BP_INCLUDE_DIR bp/include)
set(BP_UTIL_INCLUDE_DIR bpUtil/include)
set(BP_SRC_DIR bp/src)
file(GLOB_RECURSE BP_SRC_FILES "${BP_SRC_DIR}/*.cpp" "src/*.c")
file(GLOB_RECURSE BP_INCLUDE_FILES "${BP_INCLUDE_DIR}/*.h" "${BP_INCLUDE_DIR}/*.hpp")
file(GLOB_RECURSE BP_UTIL_INCLUDE_FILES "${BP_UTIL_INCLUDE_DIR}/*.h" "${BP_UTIL_INCLUDE_DIR}/*.hpp")
set(BP_SRC ${BP_SRC_FILES} ${BP_INCLUDE_FILES} ${BP_UTIL_INCLUDE_FILES})

find_package(Vulkan REQUIRED)
find_package(shaderc REQUIRED)
add_library(bp STATIC ${BP_SRC})
target_include_directories(bp
	PUBLIC ${Vulkan_INCLUDE_DIRS} ${SHADERC_INCLUDE_DIRS}
	${BP_INCLUDE_DIR} ${BP_UTIL_INCLUDE_DIR})
target_link_libraries(bp ${Vulkan_LIBRARIES} ${SHADERC_LIBRARIES})

if (Scene IN_LIST BP_MODULES)
	set(BP_SCENE_INCLUDE_DIR bpScene/include)
	set(BP_SCENE_SRC_DIR bpScene/src)
	file(GLOB_RECURSE BP_SCENE_SRC_FILES "${BP_SCENE_SRC_DIR}/*.cpp" "src/*.c")
	file(GLOB_RECURSE BP_SCENE_INCLUDE_FILES "${BP_SCENE_INCLUDE_DIR}/*.h" "${BP_SCENE_INCLUDE_DIR}/*.hpp")
	set(BP_SCENE_SRC ${BP_SCENE_SRC_FILES} ${BP_SCENE_INCLUDE_FILES})

	find_package(GLM REQUIRED)
	add_library(bpScene STATIC ${BP_SCENE_SRC})
	target_include_directories(bpScene
		PUBLIC ${GLM_INCLUDE_DIRS} ${BP_SCENE_INCLUDE_DIR})
	target_link_libraries(bpScene bp)
endif()
if (Multi IN_LIST BP_MODULES)
	set(BP_MULTI_INCLUDE_DIR bpMulti/include)
	set(BP_MULTI_SRC_DIR bpMulti/src)
	file(GLOB_RECURSE BP_MULTI_SRC_FILES "${BP_MULTI_SRC_DIR}/*.cpp" "${BP_MULTI_SRC_DIR}/*.c")
	file(GLOB_RECURSE BP_MULTI_INCLUDE_FILES "${BP_MULTI_INCLUDE_DIR}/*.h" "${BP_MULTI_INCLUDE_DIR}/*.hpp")
	set(BP_MULTI_SRC ${BP_MULTI_SRC_FILES} ${BP_MULTI_INCLUDE_FILES})

	add_library(bpMulti STATIC ${BP_MULTI_SRC})
	target_include_directories(bpMulti PUBLIC ${BP_MULTI_INCLUDE_DIR})
	target_link_libraries(bpMulti bp bpScene)
endif()
if (Qt IN_LIST BP_MODULES)
	set(BP_QT_INCLUDE_DIR bpQt/include)
	set(BP_QT_SRC_DIR bpQt/src)
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

	add_library(bpQt STATIC ${BP_QT_SRC})
	target_include_directories(bpQt PUBLIC
		${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS}
		${BP_QT_INCLUDE_DIR})
	target_link_libraries(bpQt bp Qt5::Core Qt5::Gui Qt5::Widgets)
endif()
if (View IN_LIST BP_MODULES)
	set(BP_VIEW_INCLUDE_DIR bpView/include)
	set(BP_VIEW_SRC_DIR bpView/src)
	file(GLOB_RECURSE BP_VIEW_SRC_FILES "${BP_VIEW_SRC_DIR}/*.cpp" "${BP_VIEW_SRC_DIR}/*.c")
	file(GLOB_RECURSE BP_VIEW_INCLUDE_FILES "${BP_VIEW_INCLUDE_DIR}/*.h" "${BP_VIEW_INCLUDE_DIR}/*.hpp")
	set(BP_VIEW_SRC ${BP_VIEW_SRC_FILES} ${BP_VIEW_INCLUDE_FILES})

	find_package(GLFW REQUIRED)
	add_library(bpView STATIC ${BP_VIEW_SRC})
	target_include_directories(bpView
		PUBLIC ${GLFW_INCLUDE_DIR} ${BP_VIEW_INCLUDE_DIR})
	target_link_libraries(bpView bp ${GLFW_LIBRARY})
endif()

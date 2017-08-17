# Try to find BP library
# Once done this will define
# BP_FOUND
# BP_INCLUDE_DIRS
# BP_LIBRARIES

find_package(GLFW REQUIRED)
find_package(Vulkan REQUIRED)

find_path(BP_INCLUDE_DIR bp/bp.h
	  HINTS ${BP_ROOT_DIR}/include ${BP_INCLUDE_DIR}
	  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../bp/include)

find_library(BP_LIBRARY NAMES bp
	     HINTS ${BP_ROOT_DIR}/lib ${BP_LIBRARY_DIR}
	     PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../bp/lib ${CMAKE_CURRENT_SOURCE_DIR}/../bp/cmake-build-debug)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BP DEFAULT_MSG
				  BP_LIBRARY BP_INCLUDE_DIR)
mark_as_advanced(BP_INCLUDE_DIR BP_LIBRARY)

set(BP_INCLUDE_DIRS ${GLFW_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS} ${BP_INCLUDE_DIR})
set(BP_LIBRARIES ${BP_LIBRARY} ${GLFW_LIBRARY} ${Vulkan_LIBRARIES})
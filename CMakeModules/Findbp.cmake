# Try to find bp library
# Once done this will define
# BP_FOUND
# BP_INCLUDE_DIRS
# BP_LIBRARIES

find_package(Vulkan REQUIRED)

find_path(BP_INCLUDE_DIR bp/Instance.h
	  HINTS ${BP_ROOT_DIR}/include ${BP_ROOT_DIR}/bp/include ${BP_INCLUDE_DIR}
	  PATHS ../bp/bp/include)

find_library(BP_LIBRARY NAMES bp
	     HINTS ${BP_ROOT_DIR}/lib ${BP_ROOT_DIR}/cmake-build-debug ${BP_LIBRARY_DIR}
	     PATHS ../bp/lib ../bp/cmake-build-debug)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BP DEFAULT_MSG
				  BP_LIBRARY BP_INCLUDE_DIR)
mark_as_advanced(BP_INCLUDE_DIR BP_LIBRARY)

set(BP_INCLUDE_DIRS ${BP_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})
set(BP_LIBRARIES ${BP_LIBRARY} ${Vulkan_LIBRARIES})
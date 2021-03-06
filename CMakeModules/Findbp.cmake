# Try to find bp library
# Once done this will define
# BP_FOUND
# BP_INCLUDE_DIRS
# BP_LIBRARIES

find_package(Vulkan REQUIRED)
find_package(shaderc REQUIRED)

find_path(BP_INCLUDE_DIR bp/Instance.h
	  HINTS ${BP_ROOT_DIR}/include ${BP_ROOT_DIR}/bp/include ${BP_INCLUDE_DIR}
	  PATHS ../bp/bp/include)
find_path(BP_UTIL_INCLUDE_DIR bpUtil/Event.h
	  HINTS ${BP_ROOT_DIR}/include ${BP_ROOT_DIR}/bpUtil/include ${BP_UTIL_INCLUDE_DIR}
	  PATHS ../bp/bpUtil/include)

find_library(BP_LIBRARY_RELEASE NAMES bp
	     HINTS ${BP_ROOT_DIR}/lib ${BP_ROOT_DIR}/cmake-build-release
	     PATHS ${BP_LIBRARY_DIR} ../bp/lib ../bp/cmake-build-release)
find_library(BP_LIBRARY_DEBUG NAMES bpd
	     HINTS ${BP_ROOT_DIR}/lib ${BP_ROOT_DIR}/cmake-build-debug
	     PATHS ${BP_LIBRARY_DIR} ../bp/lib ../bp/cmake-build-debug)

if(BP_LIBRARY_RELEASE AND BP_LIBRARY_DEBUG)
	set(BP_LIBRARY optimized ${BP_LIBRARY_RELEASE} debug ${BP_LIBRARY_DEBUG})
elseif(BP_LIBRARY_RELEASE)
	set(BP_LIBRARY ${BP_LIBRARY_RELEASE})
elseif(BP_LIBRARY_DEBUG)
	set(BP_LIBRARY ${BP_LIBRARY_DEBUG})
else()
	message(SEND_ERROR "bp library not found.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BP DEFAULT_MSG
				  BP_LIBRARY BP_INCLUDE_DIR)
mark_as_advanced(BP_INCLUDE_DIR BP_LIBRARY)

set(BP_INCLUDE_DIRS
    ${BP_INCLUDE_DIR}
    ${BP_UTIL_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIRS}
    ${SHADERC_INCLUDE_DIRS})
set(BP_LIBRARIES
    ${BP_LIBRARY}
    ${Vulkan_LIBRARIES}
    ${SHADERC_LIBRARIES})

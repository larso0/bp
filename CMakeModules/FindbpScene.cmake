# Try to find bpScene library
# Once done this will define
# BP_SCENE_FOUND
# BP_SCENE_INCLUDE_DIRS
# BP_SCENE_LIBRARIES

find_package(bp REQUIRED)
find_package(GLM REQUIRED)

find_path(BP_SCENE_INCLUDE_DIR bpScene/Node.h
	  HINTS ${BP_SCENE_ROOT_DIR}/include ${BP_ROOT_DIR}/bpScene/include ${BP_SCENE_INCLUDE_DIR}
	  PATHS ../bp/bpScene/include)

find_library(BP_SCENE_LIBRARY_RELEASE NAMES bpScene
	     HINTS
	     	${BP_SCENE_ROOT_DIR}/lib
	     	${BP_SCENE_ROOT_DIR}/cmake-build-release
	     	${BP_ROOT_DIR}/cmake-build-release
	     PATHS
	     	${BP_SCENE_LIBRARY_DIR}
	     	${BP_LIBRARY_DIR}
	     	../bp/lib
	     	../bp/cmake-build-release)
find_library(BP_SCENE_LIBRARY_DEBUG NAMES bpScened
	     HINTS
	     ${BP_SCENE_ROOT_DIR}/lib
	     ${BP_SCENE_ROOT_DIR}/cmake-build-debug
	     ${BP_ROOT_DIR}/cmake-build-debug
	     PATHS
	     ${BP_SCENE_LIBRARY_DIR}
	     ${BP_LIBRARY_DIR}
	     ../bp/lib
	     ../bp/cmake-build-debug)

if(BP_SCENE_LIBRARY_RELEASE AND BP_SCENE_LIBRARY_DEBUG)
	set(BP_SCENE_LIBRARY optimized ${BP_SCENE_LIBRARY_RELEASE} debug ${BP_SCENE_LIBRARY_DEBUG})
elseif(BP_SCENE_LIBRARY_RELEASE)
	set(BP_SCENE_LIBRARY ${BP_SCENE_LIBRARY_RELEASE})
elseif(BP_SCENE_LIBRARY_DEBUG)
	set(BP_SCENE_LIBRARY ${BP_SCENE_LIBRARY_DEBUG})
else()
	message(SEND_ERROR "bpScene library not found.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BP_SCENE DEFAULT_MSG
				  BP_SCENE_LIBRARY BP_SCENE_INCLUDE_DIR)
mark_as_advanced(BP_SCENE_INCLUDE_DIR BP_SCENE_LIBRARY)

set(BP_SCENE_INCLUDE_DIRS ${BP_SCENE_INCLUDE_DIR} ${BP_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
set(BP_SCENE_LIBRARIES ${BP_SCENE_LIBRARY} ${BP_LIBRARIES})

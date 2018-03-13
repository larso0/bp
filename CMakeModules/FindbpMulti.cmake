# Try to find bpMulti library
# Once done this will define
# BP_MULTI_FOUND
# BP_MULTI_INCLUDE_DIRS
# BP_MULTI_LIBRARIES

find_package(bp REQUIRED)
find_package(bpScene REQUIRED)

find_path(BP_MULTI_INCLUDE_DIR bpMulti/Compositor.h
	  HINTS ${BP_MULTI_ROOT_DIR}/include ${BP_ROOT_DIR}/bpMulti/include ${BP_MULTI_INCLUDE_DIR}
	  PATHS ../bp/bpMulti/include)

find_library(BP_MULTI_LIBRARY_RELEASE NAMES bpMulti
	     HINTS
	     	${BP_MULTI_ROOT_DIR}/lib
	     	${BP_MULTI_ROOT_DIR}/cmake-build-release
	     	${BP_ROOT_DIR}/cmake-build-release
	     PATHS
	     	${BP_MULTI_LIBRARY_DIR}
	     	${BP_LIBRARY_DIR}
	     	../bp/lib
	     	../bp/cmake-build-release)
find_library(BP_MULTI_LIBRARY_DEBUG NAMES bpMultid
	     HINTS
	     ${BP_MULTI_ROOT_DIR}/lib
	     ${BP_MULTI_ROOT_DIR}/cmake-build-debug
	     ${BP_ROOT_DIR}/cmake-build-debug
	     PATHS
	     ${BP_MULTI_LIBRARY_DIR}
	     ${BP_LIBRARY_DIR}
	     ../bp/lib
	     ../bp/cmake-build-debug)
set(BP_MULTI_LIBRARY optimized ${BP_MULTI_LIBRARY_RELEASE} debug ${BP_MULTI_LIBRARY_DEBUG})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BP_MULTI DEFAULT_MSG
				  BP_MULTI_LIBRARY BP_MULTI_INCLUDE_DIR)
mark_as_advanced(BP_MULTI_INCLUDE_DIR BP_MULTI_LIBRARY)

set(BP_MULTI_INCLUDE_DIRS ${BP_MULTI_INCLUDE_DIR} ${BP_INCLUDE_DIRS} ${BP_SCENE_INCLUDE_DIRS})
set(BP_MULTI_LIBRARIES ${BP_MULTI_LIBRARY} ${BP_LIBRARIES} ${BP_SCENE_LIBRARIES})
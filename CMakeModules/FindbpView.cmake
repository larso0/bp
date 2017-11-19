# Try to find bpView library
# Once done this will define
# BP_VIEW_FOUND
# BP_VIEW_INCLUDE_DIRS
# BP_VIEW_LIBRARIES

find_package(bp REQUIRED)
find_package(GLFW REQUIRED)

find_path(BP_VIEW_INCLUDE_DIR bpView/Window.h
	  HINTS ${BP_VIEW_ROOT_DIR}/include ${BP_ROOT_DIR}/bpView/include ${BP_VIEW_INCLUDE_DIR}
	  PATHS ../bp/bpView/include)

find_library(BP_VIEW_LIBRARY NAMES bpView
	     HINTS
	     	${BP_VIEW_ROOT_DIR}/lib
	     	${BP_VIEW_ROOT_DIR}/cmake-build-debug
	     	${BP_ROOT_DIR}/cmake-build-debug
	     PATHS
	     	${BP_VIEW_LIBRARY_DIR}
	     	${BP_LIBRARY_DIR}
	     	../bp/lib
	     	../bp/cmake-build-debug)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BP_VIEW DEFAULT_MSG
				  BP_VIEW_LIBRARY BP_VIEW_INCLUDE_DIR)
mark_as_advanced(BP_VIEW_INCLUDE_DIR BP_VIEW_LIBRARY)

set(BP_VIEW_INCLUDE_DIRS ${BP_VIEW_INCLUDE_DIR} ${BP_INCLUDE_DIRS} ${GLFW_INCLUDE_DIR})
set(BP_VIEW_LIBRARIES ${BP_VIEW_LIBRARY} ${BP_LIBRARIES} ${GLFW_LIBRARY})
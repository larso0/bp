# Try to find bpQt library
# Once done this will define
# BP_QT_FOUND
# BP_QT_INCLUDE_DIRS
# BP_QT_LIBRARIES

find_package(bp REQUIRED)

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

find_path(BP_QT_INCLUDE_DIR bpQt/Window.h
	  HINTS ${BP_QT_ROOT_DIR}/include ${BP_ROOT_DIR}/bpQt/include ${BP_QT_INCLUDE_DIR}
	  PATHS ../bp/bpQt/include)

find_library(BP_QT_LIBRARY_RELEASE NAMES bpQt
	     HINTS
	     	${BP_QT_ROOT_DIR}/lib
	     	${BP_QT_ROOT_DIR}/cmake-build-release
	     	${BP_ROOT_DIR}/cmake-build-release
	     PATHS
	     	${BP_QT_LIBRARY_DIR}
	     	${BP_LIBRARY_DIR}
	     	../bp/lib
	     	../bp/cmake-build-release)
find_library(BP_QT_LIBRARY_DEBUG NAMES bpQtd
	     HINTS
	     	${BP_QT_ROOT_DIR}/lib
	     	${BP_QT_ROOT_DIR}/cmake-build-debug
	     	${BP_ROOT_DIR}/cmake-build-debug
	     PATHS
	     	${BP_QT_LIBRARY_DIR}
	     	${BP_LIBRARY_DIR}
	     	../bp/lib
	     	../bp/cmake-build-debug)
set(BP_QT_LIBRARY optimized ${BP_QT_LIBRARY_RELEASE} debug ${BP_QT_LIBRARY_DEBUG})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BP_QT DEFAULT_MSG
				  BP_QT_LIBRARY BP_QT_INCLUDE_DIR)
mark_as_advanced(BP_QT_INCLUDE_DIR BP_QT_LIBRARY)

set(BP_QT_INCLUDE_DIRS ${BP_QT_INCLUDE_DIR} ${BP_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
set(BP_QT_LIBRARIES ${BP_QT_LIBRARY} ${BP_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets)
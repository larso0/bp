# Try to find bp library
# Once done this will define
# SHADERC_FOUND
# SHADERC_INCLUDE_DIRS
# SHADERC_LIBRARIES

find_path(SHADERC_INCLUDE_DIR shaderc/shaderc.h
	  HINTS ${SHADERC_ROOT_DIR}/include ${SHADERC_INCLUDE_DIR}
	  PATHS $ENV{VULKAN_SDK}/Include)

find_library(SHADERC_LIBRARY NAMES shaderc_combined
	     HINTS ${SHADERC_ROOT_DIR}/lib ${SHADERC_LIBRARY_DIR}
	     PATHS $ENV{VULKAN_SDK}/Lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SHADERC DEFAULT_MSG
				  SHADERC_LIBRARY SHADERC_INCLUDE_DIR)
mark_as_advanced(SHADERC_INCLUDE_DIR SHADERC_LIBRARY)

set(SHADERC_INCLUDE_DIRS ${SHADERC_INCLUDE_DIR})
set(SHADERC_LIBRARIES ${SHADERC_LIBRARY})
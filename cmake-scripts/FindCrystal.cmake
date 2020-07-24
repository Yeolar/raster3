# Find crystal
#
#  CRYSTAL_INCLUDE_DIR - where to find crystal/*.
#  CRYSTAL_LIBRARY     - List of libraries when using crystal.
#  CRYSTAL_FOUND       - True if crystal found.

if(CRYSTAL_INCLUDE_DIR)
  # Already in cache, be silent
  set(CRYSTAL_FIND_QUIETLY TRUE)
endif()

find_path(CRYSTAL_INCLUDE_DIR crystal/crystal-config.h)
find_library(CRYSTAL_LIBRARY NAMES crystal)

# handle the QUIETLY and REQUIRED arguments and set CRYSTAL_FOUND to TRUE 
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    CRYSTAL DEFAULT_MSG
    CRYSTAL_LIBRARY CRYSTAL_INCLUDE_DIR)

mark_as_advanced(CRYSTAL_LIBRARY CRYSTAL_INCLUDE_DIR)

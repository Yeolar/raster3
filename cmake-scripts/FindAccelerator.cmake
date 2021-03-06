# Find accelerator
#
#  ACCELERATOR_INCLUDE_DIR - where to find accelerator/*.
#  ACCELERATOR_LIBRARY     - List of libraries when using accelerator.
#  ACCELERATOR_FOUND       - True if accelerator found.

if(ACCELERATOR_INCLUDE_DIR)
  # Already in cache, be silent
  set(ACCELERATOR_FIND_QUIETLY TRUE)
endif()

find_path(ACCELERATOR_INCLUDE_DIR accelerator/accelerator-config.h)
find_library(ACCELERATOR_LIBRARY NAMES accelerator)

# handle the QUIETLY and REQUIRED arguments and set ACCELERATOR_FOUND to TRUE 
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ACCELERATOR DEFAULT_MSG
    ACCELERATOR_LIBRARY ACCELERATOR_INCLUDE_DIR)

mark_as_advanced(ACCELERATOR_LIBRARY ACCELERATOR_INCLUDE_DIR)

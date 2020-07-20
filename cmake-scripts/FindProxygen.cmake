# Find proxygen
#
#  PROXYGEN_INCLUDE_DIR - where to find proxygen/*.
#  PROXYGEN_LIBRARIES   - List of libraries when using proxygen.
#  PROXYGEN_FOUND       - True if proxygen found.

if(PROXYGEN_INCLUDE_DIR)
  # Already in cache, be silent
  set(PROXYGEN_FIND_QUIETLY TRUE)
endif()

find_path(PROXYGEN_INCLUDE_DIR proxygen/httpserver/HTTPServer.h)
find_library(PROXYGEN_CURL proxygencurl)
find_library(PROXYGEN_LIB proxygenlib)
find_library(PROXYGEN_SERVER proxygenhttpserver)
set(PROXYGEN_LIBRARIES ${PROXYGEN_CURL} ${PROXYGEN_LIB} ${PROXYGEN_SERVER})

# handle the QUIETLY and REQUIRED arguments and set PROXYGEN_FOUND to TRUE 
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    PROXYGEN DEFAULT_MSG
    PROXYGEN_LIBRARIES PROXYGEN_INCLUDE_DIR)

mark_as_advanced(PROXYGEN_LIBRARIES PROXYGEN_INCLUDE_DIR)

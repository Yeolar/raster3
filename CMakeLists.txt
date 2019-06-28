# Copyright (C) 2019, Yeolar

# Define project
project(pbrpc)

cmake_minimum_required(VERSION 3.2)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

# Set project version
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

# Required packages
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
find_package(GFlags REQUIRED)
find_package(Glog REQUIRED)
find_package(Folly REQUIRED)
find_package(Wangle REQUIRED)
find_package(Protobuf REQUIRED)

# Optional packages
find_package(GTest)

# Setup environment
set(CMAKE_BUILD_TYPE Release)   # Debug: -g; Release: -O3 -DNDEBUG
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")    # memcheck
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Link libraries
link_libraries(
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
    ${Boost_LIBRARIES}
    ${GFLAGS_LIBRARIES}
    ${GLOG_LIBRARIES}
    ${FOLLY_LIBRARIES}
    ${WANGLE_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

# Check and setup configurations
include(CheckCXXSourceCompiles REQUIRED)

check_cxx_source_compiles("
    #include <string.h>
    #include <errno.h>
    int main() {
        char buf[1024];
        buf[0] = 0;
        int ret = strerror_r(ENOMEM, buf, sizeof(buf));
        return ret;
    }"
    CDATA_HAVE_XSI_STRERROR_R)

if(GFLAGS_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES "gflags")

    check_cxx_source_compiles("
        #include <gflags/gflags.h>
        void foo() { google::GetArgv(); }
        int main() { return 0; }"
        CDATA_UNUSUAL_GFLAGS_NAMESPACE)

    if(CDATA_UNUSUAL_GFLAGS_NAMESPACE)
        set(CDATA_GFLAGS_NAMESPACE "google")
    else()
        set(CDATA_GFLAGS_NAMESPACE "gflags")
    endif()
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/pbrpc/pbrpc-config.h.in
    ${PROJECT_SOURCE_DIR}/pbrpc/pbrpc-config.h
)

# Binary
add_subdirectory(pbrpc)

# Test
if(GTEST_FOUND)
    enable_testing()
#    add_subdirectory(pbrpc/test)
endif()


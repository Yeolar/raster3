# Copyright (C) 2019, Yeolar

file(GLOB RASTER_SAMPLEAPP_SRCS *.cpp)

add_library(raster_sampleapp SHARED ${RASTER_SAMPLEAPP_SRCS})

install(TARGETS raster_sampleapp
    DESTINATION lib
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)

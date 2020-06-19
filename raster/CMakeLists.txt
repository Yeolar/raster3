# Copyright (C) 2019, Yeolar

set(RASTER_PROTOS
    Message.proto
)

foreach(proto ${RASTER_PROTOS})
    execute_process(COMMAND
        protoc --cpp_out=${PROJECT_SOURCE_DIR}
               --proto_path=${PROJECT_SOURCE_DIR}
               ${PROJECT_SOURCE_DIR}/raster/${proto})
endforeach()

file(GLOB RASTER_HDRS *.h)

install(FILES ${RASTER_HDRS} DESTINATION include/raster)

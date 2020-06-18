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

add_executable(raster_client
    Message.pb.cc
    RpcClient.cpp
)
target_link_libraries(raster_client)

add_executable(raster_server
    Message.pb.cc
    RpcServer.cpp
)
target_link_libraries(raster_server)


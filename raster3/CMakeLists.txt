# Copyright (C) 2019, Yeolar

set(RASTER3_PROTOS
    Message.proto
)

foreach(proto ${RASTER3_PROTOS})
    execute_process(COMMAND
        protoc --cpp_out=${PROJECT_SOURCE_DIR}
               --proto_path=${PROJECT_SOURCE_DIR}
               ${PROJECT_SOURCE_DIR}/raster3/${proto})
endforeach()

add_executable(raster3_client
    Message.pb.cc
    RpcClient.cpp
)
target_link_libraries(raster3_client)

add_executable(raster3_server
    Message.pb.cc
    RpcServer.cpp
)
target_link_libraries(raster3_server)


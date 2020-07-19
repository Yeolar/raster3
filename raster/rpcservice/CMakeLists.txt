# Copyright (C) 2019, Yeolar

set(RASTER_PROTOS
    Message.proto
)

foreach(proto ${RASTER_PROTOS})
    execute_process(COMMAND
        protoc --cpp_out=${PROJECT_SOURCE_DIR}
               --proto_path=${PROJECT_SOURCE_DIR}
               ${PROJECT_SOURCE_DIR}/raster/rpcservice/${proto})
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

install(TARGETS raster_client
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(TARGETS raster_server
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(FILES
    scripts/client.sh
    scripts/server.sh
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(DIRECTORY conf DESTINATION .)

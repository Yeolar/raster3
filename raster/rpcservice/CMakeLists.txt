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

add_executable(raster-rpcclient
    Message.pb.cc
    RpcClient.cpp
)
target_link_libraries(raster-rpcclient)

add_executable(raster-rpcserver
    Message.pb.cc
    RpcServer.cpp
)
target_link_libraries(raster-rpcserver)

install(TARGETS raster-rpcclient
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(TARGETS raster-rpcserver
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(FILES
    scripts/rpcclient.sh
    scripts/rpcserver.sh
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(DIRECTORY conf DESTINATION .)

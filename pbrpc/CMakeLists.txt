# Copyright (C) 2019, Yeolar

set(PBRPC_PROTOS
    Message.proto
)

foreach(proto ${PBRPC_PROTOS})
    execute_process(COMMAND
        protoc --cpp_out=${PROJECT_SOURCE_DIR}
               --proto_path=${PROJECT_SOURCE_DIR}
               ${PROJECT_SOURCE_DIR}/pbrpc/${proto})
endforeach()

add_executable(pbrpc_client
    Message.pb.cc
    RpcClient.cpp
)
target_link_libraries(pbrpc_client)

add_executable(pbrpc_server
    Message.pb.cc
    RpcServer.cpp
)
target_link_libraries(pbrpc_server)


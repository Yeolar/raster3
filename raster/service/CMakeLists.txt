# Copyright (C) 2019, Yeolar

add_executable(raster_client
    ../Message.pb.cc
    RpcClient.cpp
)
target_link_libraries(raster_client)

add_executable(raster_server
    ../Message.pb.cc
    ../sampleapp/GraphScheduler.cpp
    RpcServer.cpp
)
target_link_libraries(raster_server)

install(FILES ${raster_client} DESTINATION bin)
install(FILES ${raster_server} DESTINATION bin)

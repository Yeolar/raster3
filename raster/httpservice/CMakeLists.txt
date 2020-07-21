# Copyright (C) 2019, Yeolar

add_executable(raster-httpserver
    HttpHandler.cpp
    HttpServer.cpp
)
target_link_libraries(raster-httpserver)

install(TARGETS raster-httpserver
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(FILES
    scripts/httpserver.sh
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
install(DIRECTORY conf DESTINATION .)

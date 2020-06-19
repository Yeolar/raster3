# Copyright (C) 2020, Yeolar

file(GLOB RASTER_TASKFLOW_HDRS
    *.hpp
    cuda/*.hpp
    nstd/*.hpp
    utility/*.hpp)

install(FILES ${RASTER_TASKFLOW_HDRS} DESTINATION include/raster/taskflow)

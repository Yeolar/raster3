# Copyright (C) 2020, Yeolar

file(GLOB RASTER_TASKFLOW_HDRS *.hpp)
file(GLOB RASTER_TASKFLOW_CUDA_HDRS cuda/*.hpp)
file(GLOB RASTER_TASKFLOW_NSTD_HDRS nstd/*.hpp)
file(GLOB RASTER_TASKFLOW_UTILITY_HDRS utility/*.hpp)

install(FILES ${RASTER_TASKFLOW_HDRS} DESTINATION include/raster/taskflow)
install(FILES ${RASTER_TASKFLOW_CUDA_HDRS} DESTINATION include/raster/taskflow/cuda)
install(FILES ${RASTER_TASKFLOW_NSTD_HDRS} DESTINATION include/raster/taskflow/nstd)
install(FILES ${RASTER_TASKFLOW_UTILITY_HDRS} DESTINATION include/raster/taskflow/utility)

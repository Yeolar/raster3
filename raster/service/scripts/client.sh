#!/bin/sh

export LD_LIBRARY_PATH="`pwd`/lib:$LD_LIBRARY_PATH"
./bin/raster_client

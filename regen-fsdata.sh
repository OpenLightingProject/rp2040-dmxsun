#!/bin/sh

if [ ! -f makefsdata ]; then
    # Doing this outside cmake as we don't want it cross-compiled but for host
    echo Compiling makefsdata
    gcc -o makefsdata -Ilib/lwip/src/include -Isrc lib/lwip/src/apps/http/makefsdata/makefsdata.c
fi

echo Regenerating fsdata.c
./makefsdata www -f:src/fsdata.c
echo Done

#!/bin/sh

# If the makefsdata tool shipped with lwip hasn't been compiled, do so now
if [ ! -f makefsdata ]; then
    # Doing this outside cmake as we don't want it cross-compiled but for host
    echo Compiling makefsdata
    if [ "$CI" == "true" ]; then
        # Use the slower but more effective zopfli algorithm
        gcc -o makefsdata -DMAKEFS_SUPPORT_DEFLATE=1 -DMAKEFS_SUPPORT_DEFLATE_ZOPFLI=1 -Ilib/lwip/src/include -Isrc lib/lwip/src/apps/http/makefsdata/makefsdata.c -lz -lzopfli
    else
        # During development, just use the blazing fast zlib
        gcc -o makefsdata -DMAKEFS_SUPPORT_DEFLATE=1 -DMAKEFS_SUPPORT_DEFLATE_ZLIB=1 -Ilib/lwip/src/include -Isrc lib/lwip/src/apps/http/makefsdata/makefsdata.c -lz
    fi
fi

echo Regenerating fsdata.c ...
if [ "$CI" == "true" ]; then
    # Use the slower but more effective zopfli algorithm
    ./makefsdata www -defl:400 -11 -f:src/fsdata.c
else
    # During development, just use the blazing fast zlib
    ./makefsdata www -defl:6 -11 -f:src/fsdata.c
fi
echo Done

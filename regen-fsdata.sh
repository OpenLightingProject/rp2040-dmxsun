#!/bin/sh

if [ ! -f makefsdata ]; then
    # Doing this outside cmake as we don't want it cross-compiled but for host
    echo Compiling makefsdata
    #gcc -o makefsdata -DMAKEFS_SUPPORT_BROTLI -lbrotlienc -lbrotlidec -Ilib/lwip/src/include -Isrc lib/lwip/src/apps/http/makefsdata/makefsdata.c
    gcc -o makefsdata -DMAKEFS_SUPPORT_DEFLATE=1 -DMAKEFS_SUPPORT_DEFLATE_ZLIB=1 -lz -Ilib/lwip/src/include -Isrc lib/lwip/src/apps/http/makefsdata/makefsdata.c
fi

echo Regenerating fsdata.c
#./makefsdata www -brotli:11 -11 -f:src/fsdata.c
./makefsdata www -defl:9 -11 -f:src/fsdata.c
echo Done

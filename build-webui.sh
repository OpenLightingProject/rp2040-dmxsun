#!/bin/sh

echo Building web ui ...
cd wwwsrc
npm i
export GENERATE_SOURCEMAP=false
npm run build
cd ..
cp -rv wwwsrc/build/* www/
echo Done

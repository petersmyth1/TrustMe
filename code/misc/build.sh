#!/bin/bash
mkdir ../build/
pushd ../build/
echo "******************************************************"
echo ""
clang++ -g -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -I../Leap/include ../src/test.cpp -o  TrustMe.app ../Leap/lib/libLeap.dylib
install_name_tool -change @loader_path/libLeap.dylib ../Leap/lib/libLeap.dylib TrustMe.app
./TrustMe.app
echo ""
echo "******************************************************"
popd

#-I/usr/local/include -L/usr/local/lib -lSDL2

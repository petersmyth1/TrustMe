OS := $(shell uname)
ARCH := $(shell uname -m)

ifeq ($(OS), Linux)
  ifeq ($(ARCH), x86_64)
    LEAP_LIBRARY := ../lib/x64/libLeap.so -Wl,-rpath,../lib/x64
  else
    LEAP_LIBRARY := ../lib/x86/libLeap.so -Wl,-rpath,../lib/x86
  endif
else
  # OS X
  LEAP_LIBRARY := ../Leap/lib/libLeap.dylib
endif

TrustMe: ../src/test.cpp
	$(CXX) -Wall -g -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -I../Leap/include ../src/test.cpp -o TrustMe  $(LEAP_LIBRARY)
ifeq ($(OS), Darwin)
	install_name_tool -change @loader_path/libLeap.dylib ../Leap/lib/libLeap.dylib TrustMe.app
endif

clean:
	rm -rf TrustMe TrustMe.dSYM

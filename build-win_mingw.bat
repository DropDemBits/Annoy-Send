cd build-win
mkdir cmake-out
cd cmake-out
cmake.exe -G "MinGW Makefiles" ..
make.exe
copy annoy.exe ../../annoy.exe

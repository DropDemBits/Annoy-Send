cd build-win
mkdir cmake-out
cd cmake-out
cmake.exe -G "NMake Makefiles" ..
nmake.exe
copy annoy.exe ../../annoy.exe

cd build-nix
mkdir cmake-out
cd cmake-out
cmake -G "Unix Makefiles" ..
make
cp annoy ../../annoy

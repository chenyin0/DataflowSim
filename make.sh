#!/usr/bin/bash
cd ./build
rm CMakeCache.txt
cmake ..
make -j10
cd ..
#./dfsim_ogbn --dataset cora --arch hygcn

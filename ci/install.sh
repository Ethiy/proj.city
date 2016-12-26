#! /usr/bin/env bash

mkdir 3rdParty && cd 3rdParty
curl -O https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/lib3ds/lib3ds-1.3.0.zip
unzip lib3ds-1.3.0.zip
rm lib3ds-1.3.0.zip
cd lib3ds-1.3.0/
./configure
make -j4 all && make install

cd ..
git clone https://github.com/CGAL/cgal.git
cd cgal
git checkout releases/CGAL-4.9
mkdir build && cd build
cmake ..
make -j4 all
make install && make install_FindCGAL
cd ..
#!/bin/bash
test $DEBUG && set -x
set -e

g++ --version;
clang --version; 
cmake --version;
echo "ninja: $(ninja --version)"

# Build LittleEngine3D
build() {
	CONFIG=$1
	echo "== Building LittleEngine3D | $CONFIG..."
	[[ ! -d out/$CONFIG ]] && mkdir -p out/$CONFIG
	cmake -G Ninja $2 -B $3/$CONFIG -DCMAKE_BUILD_TYPE=$CONFIG -DUPDATE_SUBMODULES=0 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCI_BUILD=1
	cmake --build out/$CONFIG $BUILD_FLAGS
}

[[ -z "$CONFIGS" ]] && CONFIGS=Release

for CONFIG in $CONFIGS; do
	build $CONFIG $1 $2
done

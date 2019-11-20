#!/bin/bash
test $DEBUG && set -x
set -e

echo -e "\n-- Tools:\n"
g++ --version;
clang --version; echo
cmake --version;
echo -e "\nninja: $(ninja --version)\n"

# Build LittleEngine3D
build() {
	CONFIG=$1
	echo "== Building LittleEngine3D | $CONFIG..."
	[[ ! -d out/$CONFIG ]] && mkdir -p out/$CONFIG
	cmake -G Ninja $2 -B $3/$CONFIG -DCMAKE_BUILD_TYPE=$CONFIG -DUPDATE_SUBMODULES=0 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCI_BUILD=1
	ninja -v -C out/$CONFIG $BUILD_FLAGS
}

[[ -z "$CONFIGS" ]] && CONFIGS=Release

for CONFIG in $CONFIGS; do
	build $CONFIG $1 $2
done

#!/bin/bash

OPENALPR_INCLUDE_DIR=$(pwd)/../../openalpr
OPENALPR_LIB_DIR=$(pwd)/../../build/openalpr

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.:${OPENALPR_LIB_DIR}

g++ -Wall -L${OPENALPR_LIB_DIR} -I${OPENALPR_INCLUDE_DIR} -shared -fPIC -o libopenalprgo.so openalprgo.cpp -lopenalpr

(cd openalpr && go install)

go run main.go

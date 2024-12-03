#!/bin/bash

sudo rm -rf build
mkdir build
cd build
cmake .. && make

if [[ $? = 0 ]]; then
    echo "success"
else
    # then try with my docker
    cd ../..
    /thirdparty/docker/run_docker.sh golden-sample:latest "cd /workspace/application_tool && rm -rf build && mkdir build && cd build && cmake .. && make"
    cd -
fi


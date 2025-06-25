#!/bin/bash

docker load -i ARChessCube.tar

xhost +local:root

docker run --rm -it \
    --device-cgroup-rule='c 81:* rmw' \
    --device-cgroup-rule='c 189:* rmw' \
    -v /dev:/dev \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v $HOME/.Xauthority:/root/.Xauthority \
    --net=host \
    --privileged \
    archesscube
	
xhost -local:root
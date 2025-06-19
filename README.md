# ARChessCube - Augmented Reality Chessboard Cube Project

An OpenCV-based application that projects a 3D cube onto a detected chessboard pattern using camera input.

## Features
- Real-time chessboard detection
- 3D cube projection with perspective
- Interactive controls for moving and rotating the cube

## Prerequisites
- OpenCV (≥ 4.0)
- CMake (≥ 3.10)
- C++17 compatible compiler
- Docker (optional)

## Build with CMake

```bash
# In project root directory
mkdir build && cd build
cmake ..
make
```
The executable will be created at build/ARChessCube



## Keyboard Controls
- W/A/S/D: Move cube along X/Y axes

- E/F: Move cube along Z axis

- I/J/K/L: Rotate cube around X/Z axes

- U/O: Rotate cube around Y axis

- ESC: Exit application

## Docker Setup
1. Allow X11 access (run once per session)
```bash
xhost +local:root
```
2. Build the Docker image
```bash
docker build -t archesscube .
```
3. Run the container
```bash
docker run --rm \
    --privileged \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --device=/dev/video0 \
    archesscube
```
Troubleshooting
Camera Issues:
bash
# Try different video devices if /dev/video0 doesn't work
ls /dev/video*
X11 Display Issues:
bash
# Verify DISPLAY variable
echo $DISPLAY  # Should return :0 or similar

# Add Xauthority if needed
docker run ... -v "$HOME/.Xauthority:/root/.Xauthority:rw" ...
Build Issues:


# If CMake can't find OpenCV
sudo apt-get install libopencv-dev
## File Structure
```text
ARChessCube/
├── CMakeLists.txt
├── Dockerfile
├── include/
│   └── util.h
├── src/
│   ├── main.cpp
│   └── util.cpp
└── README.md
```
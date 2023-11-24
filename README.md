# VectorGraphicViewer

Little toy engine to help me visually explain vector graphics to my colleagues.

## Setup

```sh
# Clone with all submodules
git clone --recursive https://github.com/mean-ui-thread/VectorGraphicViewer.git

# Download build dependencies
brew install cmake ninja

cd VectorGraphicViewer

# generate the debug project
cmake --preset ninja-debug

# generate the release project
cmake --preset ninja-release
```

## Setup WebAssembly (Optional)

To build the viewer in Web Assembly for the web, You'll need to download and setup Emscripten

```sh
# Get the emsdk repo
git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory
cd emsdk

# Fetch the latest version of the emsdk (not needed the first time you clone)
git pull

# Download and install the latest SDK tools.
./emsdk install latest

# Make the "latest" SDK "active" for the current user. (writes .emscripten file)
./emsdk activate latest

# Activate PATH and other environment variables in the current terminal
source ./emsdk_env.sh
```

Next step is to generate the Emscripten project:

```sh
cd VectorGraphicViewer

# generate the debug project
cmake --preset emscripten-ninja-debug

# generate the release project
cmake --preset emscripten-ninja-release
```

## Build

```sh
cd VectorGraphicViewer

# build the debug project
cmake --build --preset ninja-debug

# build the release project
cmake --build --preset ninja-release
```

## Build WebAssembly (Optional)

```sh
cd VectorGraphicViewer

# generate the debug project
cmake --build --preset emscripten-ninja-debug

# generate the release project
cmake --build --preset emscripten-ninja-release
```

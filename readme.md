# VRSFML — Vittorio Romeo's SFML Fork

> *(This is a fork of SFML by Vittorio Romeo.)*

- [**Major changes and design rationale**](DESIGN.md)

- [**Upstream SFML repository**](https://github.com/SFML/SFML)

## How to build

### Windows

1. Get and install [MSYS2](https://www.msys2.org/)

2. Open a MSYS shell (see [MSYS2 Environments](https://www.msys2.org/docs/environments/)) and run the following command:

    ```bash
    pacman -S mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-clang-tools-extra mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake git mingw-w64-ucrt-x86_64-lld mingw-w64-ucrt-x86_64-ccache mingw-w64-clang-x86_64-ccache
    ```

3. Open a UCRT64 shell (see [MSYS2 Environments](https://www.msys2.org/docs/environments/))

4. Clone this repository:

    ```bash
    git clone https://github.com/vittorioromeo/VRSFML
    ```

5. Enter the directory and run CMake with one of the presets:

    ```bash
    cd VRSFML
    cmake --preset vrdev_clang
    ```

6. Enter the preset build directory and run `ninja`:

    ```bash
    cd build_vrdev_clang
    ninja
    ```

7. If the build succeeded, you should now be able to run examples directly from the build directory using the `run_example.sh` script:

    ```bash
    ../run_example.sh island
    ```

8. For Emscripten support, run the following command:

    ```bash
    pacman -S mingw-w64-clang-x86_64-ninja mingw-w64-clang-x86_64-clang mingw-w64-clang-x86_64-clang-tools-extra mingw-w64-clang-x86_64-gcc mingw-w64-clang-x86_64-cmake mingw-w64-clang-x86_64-emscripten
    ```

9. Open a CLANG64 shell (UCRT64 is bugged with Emscripten at the moment) (see [MSYS2 Environments](https://www.msys2.org/docs/environments/))

10. In the main repository directory, run:

    ```bash
    cmake --preset vrdev_emcc
    cd build_vrdev_emcc
    ninja
    ```

11. If the build succeeded, you should now be able to run examples directly from the build directory using the `run_emscripten_example.sh` script:

    ```bash
    ../run_emscripten_example.sh island
    ```


### Ubuntu 24.04

> Contributed by @menuet.

```
COMPILER=gcc-14
PRESET=vrdev_gcc

# Install necessary tools and dependencies
sudo apt-get update
sudo apt-get install -y --no-install-recommends nodejs npm git mesa-utils lld
sudo apt-get install -y --no-install-recommends libxrandr-dev libxi-dev libxcursor-dev libglfw3-dev libudev-dev libfreetype-dev libogg-dev libvorbis-dev libflac-dev
sudo npm install -g setup-cpp@v0.38.3
sudo setup-cpp --compiler ${COMPILER} --cmake true --ninja true --ccache true --make true

# Export env vars for using tools
sudo -s
source .cpprc
exit
source .cpprc

# Verify tools availability
glxinfo | head -n10
git --version
g++ --version
cmake --version
ninja --version
ccache --version

# Create a working directory
mkdir -p work
cd work

# Clone the repo
git clone https://github.com/vittorioromeo/VRSFML.git

# Cmake Configure and Build
cd VRSFML
git checkout try-fix-build
cmake --preset ${PRESET}
cmake --build build_${PRESET}
```

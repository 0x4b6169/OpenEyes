# OpenPlaya

### About
A place where you can't phish.

### Build Requirements / Recommendations
- Clang Apple version 13.0.0 (clang-1300.0.29.30) Target: x86_64-apple-darwin21.1.0
- [CMake v3.22.1](https://cmake.org/download/)
- [FFMpeg v4.4.1](https://www.ffmpeg.org/download.html)
- [Doxygen v1.9.2](https://www.doxygen.nl/manual/install.html#install_src_unix) (Optional)
  - [Graphviz v2.50.0](https://www.graphviz.org/download/) (Optional)

### Build, Compile, and Run the app
- **Generate make config** -`cmake -S . -B build` - generates Makefiles for the app from project root dir
- **Compile source and generate executables** - `make -C build` - runs make in the build dir
- **Run the app** - `scripts/run-example.sh` - runs an example mp4

### Potential Setup Errors:
- cmake not found
  - Add cmake to your classpath - `export PATH="/Applications/CMake.app/Contents/bin":"$PATH"`
- Stdio.h not found 
  - On MacOS, try opening Xcode, install necessary components, then 
      delete CMakeCache.txt and rebuild
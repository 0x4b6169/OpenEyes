## Build (MacOS) 

### Requirements
- Clang Apple version 13.0.0 (clang-1300.0.29.30) Target: x86_64-apple-darwin21.1.0
- [CMake v3.22.1](https://cmake.org/download/)
- [FFMpeg v4.4.1](https://www.ffmpeg.org/download.html)
- [Doxygen v1.9.2](https://www.doxygen.nl/manual/install.html#install_src_unix) (Optional)
    - [Graphviz v2.50.0](https://www.graphviz.org/download/) (Optional)

Install the above and things should just work. After installing FFMpeg, you should be able to 
navigate to /usr/lib/Cellar/ffmpeg/... until you find your static lib files. 


### Potential Setup Errors:
- cmake not found
  - Add cmake to your classpath - `export PATH="/Applications/CMake.app/Contents/bin":"$PATH"`
- Stdio.h not found
  - On MacOS, try opening Xcode, install necessary components, then
    delete CMakeCache.txt and rebuild
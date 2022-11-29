# OpenPlaya

__Please note that all code currently is functional for MacOS and currently exists in src/main.c. The GitHub user also was messed up due to a bug that I found in GitHub's email logic..__

### About
A place where you can't phish. This tool is a PoC to capture the screen for further processing. At this point, the tool is only capable of recording the screen with FFMpeg and displaying the recording in a window managed by SDL2.

### Build Steps (Navigate to /docs to read md file, hosting Doxygen files soon)
- @subpage Build-MacOS.md
- @subpage Build-LinuxMintUbuntu.md

### Build, Compile, and Run the app
- **Generate make config** -`cmake -S . -B build` - generates Makefiles for the app from project root dir
- **Compile source and generate executables** - `make -C build` - runs make in the build dir
- **Run the app** - `./scripts/run-example.sh` - runs an example mp4

### Potential Setup Errors:
- cmake not found
  - Add cmake to your classpath - `export PATH="/Applications/CMake.app/Contents/bin":"$PATH"`
- Stdio.h not found 
  - On MacOS, try opening Xcode, install necessary components, then 
      delete CMakeCache.txt and rebuild

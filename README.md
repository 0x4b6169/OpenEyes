# OpenPlaya

### About
A place where you can't phish.

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
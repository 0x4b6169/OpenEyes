cmake_minimum_required(VERSION 3.21...3.22)

include(CMakePrintHelpers)

project(OpenPlaya
	VERSION 0.0.1
	LANGUAGES C)

# The project version number.
set(VERSION_MAJOR   0   CACHE STRING "Project major version number.")
set(VERSION_MINOR   0   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   1   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

	# Use stdlib
	set(CMAKE_C_EXTENSIONS OFF)

	# Support folders in IDEs
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	# Test only for the main app
	# include(CTest)

	# Docs only for the main app
	find_package(Doxygen OPTIONAL_COMPONENTS dot)
	if (Doxygen_FOUND)
		add_subdirectory(docs)
	else()
		message(STATUS "Doxygen not found, not building docs")
	endif()

	# Create build dir for compile-time
	if(EXISTS "build")
		message(STATUS "Build dir already exists")
	else()
		message(STATUS "Created build dir")
		file(MAKE_DIRECTORY "build")
	endif()

	# Create output dir for run-time
	if(EXISTS "output")
		message(STATUS "Output dir already exists")
	else()
		message(STATUS "Created output dir")
		file(MAKE_DIRECTORY "output")
	endif()

endif()

# Fetch Content added as of 3.11, downloads during config step
include(FetchContent)

# The list of subdirectories where a CMakeLists.txt file exists to compile code prior to configuring the executable
add_subdirectory(lib)
add_subdirectory(src)

# Tests for the main app
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
	add_subdirectory(tests)
endif()

# Configuration for the executable files and linking dependencies
file(GLOB_RECURSE SOURCES
		src/*.h
		src/*.c)

add_executable(${PROJECT_NAME}
		${SOURCES})

# Include lib headers
target_include_directories(${PROJECT_NAME} PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR})

# Link libs
target_link_libraries(${PROJECT_NAME} PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} unity)

# General Notes
	* CMakeLists.txt file is placed at the source of the project you want to build.
	* CMakeLists.txt is placed at the root of the source tree of any application, library it will work for.
	If there are multiple modules, and each module can be compiled and built separately, CMakeLists.txt can be inserted into the sub folder.
	* .cmake files can be used as scripts, which runs cmake command to prepare environment pre-processing or split tasks which can be written outside of CMakeLists.txt.
	* .cmake files can also define modules for projects. These projects can be separated build processes for libraries or extra methods for complex, multi-module projects.

# Some Commonly Used Commands

    * message: prints given message
    * cmake_minimum_required: sets minimum version of cmake to be used
    * add_executable: adds executable target with given name
    * add_library: adds a library target to be build from listed source files
    * add_subdirectory: adds a subdirectory to build

# Variables

Some of the variables can be seen as below, these are predefined according to root folder:

    * CMAKE_BINARY_DIR: Full path to top level of build tree and binary output folder, by default it is defined as top level of build tree.
    * CMAKE_HOME_DIRECTORY: Path to top of source tree
    * CMAKE_SOURCE_DIR: Full path to top level of source tree.
    * CMAKE_INCLUDE_PATH: Path used to find file, path


	Variable values can be accessed with ${<variable_name>}.

	message("CXX Standard: ${CMAKE_CXX_STANDARD}")
	set(CMAKE_CXX_STANDARD 14)

# List

	A list of elements represented as a string by concatenating elements separated by semi-column ‘;’.

	set(files a.txt b.txt c.txt)
	# sets files to "a.txt;b.txt;c.txt"

	In order to access the list of values you can use foreach command of CMake as following:

	foreach(file ${files})
		message("Filename: ${file}")
	endforeach()

# Basic Example

	|__ CMakeLists.txt
	|__ main.cpp


	--> CMakeList.txt
		cmake_minimum_required(VERSION 3.9.1)
		project(CMakeHello)
		add_executable(cmake_hello main.cpp)

	> cmake CMakeLists.txt
	> make all

# More Example and Useful Tips

	set(CMAKE_CXX_STANDARD 14)

-----------------

	# UNIX, WIN32, WINRT, CYGWIN, APPLE are environment variables as flags set by default system
	if(UNIX)
		message("This is a ${CMAKE_SYSTEM_NAME} system")
	elseif(WIN32)
		message("This is a Windows System")
	endif()
	# or use MATCHES to see if actual system name
	# Darwin is Apple's system name
	if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
		message("This is a ${CMAKE_SYSTEM_NAME} system")
	elseif(${CMAKE_SYSTEM_NAME} MATCHES Windows)
		message("This is a Windows System")
	endif()

------------------

	--> in CMakeList.txt
	add_definitions(-DCMAKEMACROSAMPLE="Windows PC")

	then in your source code you can use it like this
	#ifndef CMAKEMACROSAMPLE
		#define CMAKEMACROSAMPLE "NO SYSTEM NAME"
	#endif

------------------

	some useful path defines

	* CMAKE_RUNTIME_OUTPUT_DIRECTORY or EXECUTABLE_OUTPUT_PATH.
	* LIBRARY_OUTPUT_PATH or MAKE_LIBRARY_OUTPUT_DIRECTORY
	* CMAKE_ARCHIVE_OUTPUT_DIRECTORY or ARCHIVE_OUTPUT_PATH

	then you can set them like
	> set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Building Library

	Building Library with Target: If you just want to build these files together with main.cpp,
	you can just add source files next to add_executable command

	> add_executable(cmake_hello main.cpp lib/math/operations.cpp lib/math/operations.hpp)

	As an alternate, you can create a variable named ${SOURCES} as a list to include target sources.
	> set(SOURCES main.cpp
            lib/math/operations.cpp
            lib/math/operations.hpp)
	add_executable(cmake_hello ${SOURCES})

---------------

	Building Library Separate than Target

		* set LIBRARY_OUTPUT_PATH
		* add_library command as SHARED or STATIC
		* target_link_libraries to target (cmake_hello)

		example (create a shared lib first, called math and use it while compiling main that uses math in it)
		(main.cpp includes #include "lib/math/operations.hpp")

		cmake_minimum_required(VERSION 3.9.1)
		project(CMakeHello)
		set(CMAKE_CXX_STANDARD 14)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
		set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
		set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
		message(${CMAKE_BINARY_DIR})
		add_library(math SHARED lib/math/operations.cpp)
		#add_library(math STATIC lib/math/operations.cpp)
		add_executable(cmake_hello main.cpp)
		target_link_libraries(cmake_hello math)

---------------

	Build Library as Sub-Module CMake
	for the same example above, we can deperately build math lib and use seperate CMakeList.txt

	for lib/math/CMakeList.txt

	> cmake_minimum_required(VERSION 3.9.1)
	set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
	add_library(math SHARED operations.cpp)

	> main CMakeList.txt
	cmake_minimum_required(VERSION 3.9.1)
	project(CMakeHello)
	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
	message(${CMAKE_BINARY_DIR})
	add_subdirectory(lib/math)
	add_executable(cmake_hello main.cpp)
	target_link_libraries(cmake_hello math)

# Finding Library

	Finding Existing Library with CMake

	We can use, cmake’s find_package command to check if library exists before building executable.

	find_package(Boost 1.66)
	# Check for libray, if found print message, include dirs and link libraries.
	if(Boost_FOUND)
		message("Boost Found")
		include_directories(${Boost_INCLUDE_DIRS})
		target_link_libraries(cmake_hello ${Boost_LIBRARIES})
	elseif(NOT Boost_FOUND)
		error("Boost Not Found")
	endif()

	So what happens, if a library is in a custom folder and outside of source tree.

	# elseif case can be
	elseif(NOT Boost_FOUND)
	message("Boost Not Found")
		include_directories(/Users/User/Projects/libraries/include)
		link_directories(/Users/User/Projects/libraries/libs)
		target_link_libraries(cmake_hello Boost)
	endif()

# Change Compiler and Linker for Build

	set(CMAKE_SYSTEM_NAME Linux)
	set(CMAKE_SYSTEM_PROCESSOR arm)
	set(CMAKE_SYSROOT /home/devel/rasp-pi-rootfs)
	set(CMAKE_STAGING_PREFIX /home/devel/stage)
	set(tools /home/devel/gcc-4.7-linaro-rpi-gnueabihf)
	set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gnueabihf-gcc)
	set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabihf-g++)
	set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Setting Compiler Flags

	set(CMAKE_CXX_FLAGS "-std=c++0x -Wall")
	# suggested way is to keep previous flags in mind and append new ones
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
	# Alternatively, you can use generator expressions, which are conditional expressions. Below says that, if compiper is c++ then set it to c++11
	add_compile_options("$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:-std=c++11>")

# Set Source File Properties
	This is a complex property of CMake if there are multiple targets, it can be needed to change one target’s certain behavior. In case, you would like to build main.cpp with C++11 and if you building only library, you may want to build it with C++14. In such cases, you may want to configure certain source’s properties with using set_source_files_properties command like below:

	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/*.cpp PROPERTIES COMPILE_FLAGS "-std=c++11")

# Debug/Release folders and options

	$ cmake -DCMAKE_BUILD_TYPE=Debug -H.  -Bbuild/Debug
	$ cmake -DCMAKE_BUILD_TYPE=Release -H. -Bbuild/Release

	CMAKE_BUILD_TYPE is accessible inside CMakeLists.txt. You can easily check for build type in CMakeLists.txt
	if(${CMAKE_BUILD_TYPE} MATCHES Debug)
		message("Debug Build")
	elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
		message("Release Build")
	endif()

# Examples

## dynamic lib

.
├── lib
│   ├── CMakeLists.txt
│   ├── mylibrary.c
│   └── mylibrary.h
└── project
    ├── CMakeLists.txt
    └── main.c


lib/CMakeLists.txt -->
    cmake_minimum_required(VERSION 3.6)
    project(mylibrary_project)

    # CMake instructions to make the static lib

    add_library( mylibrary SHARED
                    mylibrary.c )

project/CMakeLists.txt -->
    cmake_minimum_required(VERSION 3.6)
    project(mylibrary_project)

    include_directories(${CMAKE_SOURCE_DIR}/../lib)

    add_executable(executable main.c)
    target_link_libraries(executable ${CMAKE_SOURCE_DIR}/../lib/libmylibrary.so)


## static lib

.
├── lib
│   ├── CMakeLists.txt
│   ├── mylibrary.c
│   └── mylibrary.h
└── project
    ├── CMakeLists.txt
    └── main.c


lib/CMakeLists.txt -->
    cmake_minimum_required(VERSION 3.6)
    project(mylibrary_project)

    # CMake instructions to make the static lib

    add_library( mylibrary STATIC
                    mylibrary.c )


project/CMakeLists.txt -->
    cmake_minimum_required(VERSION 3.6)
    project(mylibrary_project)

    include_directories(${CMAKE_SOURCE_DIR}/../lib)

    add_executable(executable main.c)
    target_link_libraries(executable ${CMAKE_SOURCE_DIR}/../lib/libmylibrary.a)


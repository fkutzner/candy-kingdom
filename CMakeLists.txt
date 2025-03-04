# Candy CMake files
#
#
# I. Building
#
# 1. Create a directory X, e.g. build/ and change to X
# 2. Invoke cmake (see below)
# 3. Run make
#
# The candy binary will be generated in X/, while the test binaries are placed in
# X/testsrc/$TESTMODULENAME (e.g. testsrc/gates/gates_tests). For running the tests,
# simply run ctest (if you wish to see stdout, run ctest --output-on-error (showing
# the test output only when it fails) or ctest --verbose
#
#
# To build the documentation, run
#   make doxygen
# or
#   cmake --build . --target doxygen
#
# The documentation is generated in doc/ within your build directory.
#
# II. Invoking cmake
#
# Examples:
#
#  Generate a debug build:
#    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug path/to/sources
#
#  Generate a sanitizer build:
#    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug -DCANDY_ENABLE_LLVM_SANITIZER=1 path/to/sources
#  Run with symbolizer:
#    export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
#    ASAN_OPTIONS=symbolize=1 ./candy ...
#
#  Generate a release build:
#    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Release path/to/sources
#
#  Generate an Xcode IDE project (sources still remain at their original place):
#    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug -G Xcode path/to/sources
#
#  Generate an Eclipse IDE project (sources still remain at their original place):
#    cmake -G"Eclipse CDT4 - Unix Makefiles"
#  (see https://cmake.org/Wiki/Eclipse_CDT4_Generator)
#
#
#
# III. Project structure of candy
#
#  lib/         external libraries
#  src/         source code
#    candy/     candy source code
#  testsrc/     test source code
#    candy/     candy test source code
#      ...      one directory for each module under test



project(Candy)
cmake_minimum_required(VERSION 3.5)


### Options

# Enable LLVM coverage
option(CANDY_ENABLE_COVERAGE "Enable coverage analysis" OFF)

# Enable LLVM address sanitizer
option(CANDY_ENABLE_LLVM_SANITIZER "Enable LLVM address sanitizer" OFF)

# Enable LLVM undefined-behavior, address and leak sanitizer
option(CANDY_ENABLE_LLVM_SANITIZER_FULL "Enable LLVM address, memory and undefined-behavior sanitizer" OFF)

# Enable address sanitizer
option(CANDY_ENABLE_ADDRESS_SANITIZER "Just enable address sanitizer" OFF)

# Enable -march=native - if set to OFF, at least SSE 4.1 will still be enabled.
option(CANDY_ENABLE_NATIVEARCH "Enable -march=native" OFF)

option(CANDY_ENABLE_STAREXEC_ARCH "Optimize for StarExec" OFF)

# Enable Sonification
option(CANDY_ENABLE_SONIFICATION "Enable the Glucose sonification" OFF)

# Enable Static Linking
option(CANDY_ENABLE_STATIC_LINKING "Enable static linking" OFF)

# Enable checked casts in debug mode
option(CANDY_ENABLE_DEBUG_CHECKEDCASTS "Enable checked casts in debug mode" ON)

# Disable RTTI and defines CANDY_HAS_NO_RTTI.
# WARNING: Enable this only AFTER having thoroughly tested the code. In some places
# RTTI is needed for testing (and only in places where it is also safe to use a
# reinterpret_cast - testing interfaces which document the single allowed dynamic type
# of the pointer getting casted).
# This changes the code to use reinterpret_cast instead of dynamic_cast via
# preprocessor directives.
#
# TODO: Remove this option as soon as a dynamic_cast cannot safely be replaced
# by a reinterpret_cast.
option(CANDY_DISABLE_RTTI OFF "Disable RTTI and defines CANDY_HAS_NO_RTTI. Use with caution.")


### Using ctest as a test

enable_testing()

### Platform feature checks
INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(getrlimit HAVE_GETRLIMIT)
CHECK_FUNCTION_EXISTS(setrlimit HAVE_SETRLIMIT)

if(HAVE_GETRLIMIT AND HAVE_SETRLIMIT)
  add_definitions(-D CANDY_HAVE_RLIMIT)
endif()


### Add doxygen targets
add_subdirectory(doc)

### Compiler flags

if(MSVC)
  set(CMAKE_CXX_FLAGS "/W3 /D __STDC_LIMIT_MACROS /D __STDC_FORMAT_MACROS /EHsc /F 134217728")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /D NDEBUG /Ox")
else()
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-parentheses -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-inline -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Wall -flto -O3")
endif()

if(CANDY_ENABLE_DEBUG_CHECKEDCASTS)
  add_definitions(-D DEBUG_CHECK_CASTS)
endif()

if(CANDY_ENABLE_COVERAGE)
  if(MSVC)
    message(FATAL_ERROR "Coverage not supported for MSVC.")
  endif()
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")
  SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} --coverage")
endif()

if(CANDY_ENABLE_LLVM_SANITIZER)
  if(MSVC)
    message(FATAL_ERROR "CANDY_ENABLE_LLVM_SANITIZER is not yet supported for MSVC")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address,undefined")
endif()

if(CANDY_ENABLE_LLVM_SANITIZER_FULL)
  if(MSVC)
    message(FATAL_ERROR "CANDY_ENABLE_LLVM_SANITIZER_FULL is not yet supported for MSVC")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address,memory,undefined")
endif()

if(CANDY_ENABLE_ADDRESS_SANITIZER)
  if(MSVC)
    message(FATAL_ERROR "CANDY_ENABLE_ADDRESS_SANITIZER is not yet supported for MSVC")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
endif()

if(CANDY_DISABLE_RTTI)
  message("WARNING: Use CANDY_DISABLE_RTTI only for the most optimized builds, since the test system relies on it")
  add_definitions(-DCANDY_HAS_NO_RTTI)
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
  endif()
endif()

if(CANDY_ENABLE_STATIC_LINKING)
  message("Static linking is enabled.")
  SET(BUILD_SHARED_LIBRARIES OFF)

  if (NOT MSVC)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
  endif()
endif()

if (CANDY_ENABLE_NATIVEARCH)
  message("Compiling for the processor architecture of this machine.")
  if(MSVC)
    message(FATAL_ERROR "CANDY_ENABLE_NATIVEARCH is not yet supported for MSVC")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
elseif (CANDY_ENABLE_STAREXEC_ARCH)
  message("Compiling for the processor architecture of StarExec Cluster.")
  if(MSVC)
    message(FATAL_ERROR "CANDY_ENABLE_STAREXEC_ARCH is not supported for MSVC")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=corei7-avx")
else ()
  message("Compiling for processors supporting at least SSE 4.1.")
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
  endif()
endif ()

### Additional Preprocessor Defintions

if(CANDY_ENABLE_SONIFICATION)
  if (MSVC)
    message(FATAL_ERROR "CANDY_ENABLE_SONIFICATION is not yet supported for MSVC")
  endif()
  add_definitions(-DSONIFICATION)
endif()

### Find libraries

find_package(ZLIB)
if (${ZLIB_FOUND})
   include_directories(${ZLIB_INCLUDE_DIRS})
else (${ZLIB_FOUND})
   message(FATAL_ERROR "Could not find zlib.")
endif (${ZLIB_FOUND})


### Custom functions


# This function is responsible for executable post-processing, e.g. running dsymutil.
function(candy_executable_customizations targetname)
  if (ENSURE_DSYM)
    add_custom_command(TARGET ${targetname} POST_BUILD COMMAND dsymutil Debug/${targetname})
  endif()
endfunction()



### Set up include directories

include_directories(${PROJECT_SOURCE_DIR}/src)
# for refactoring purposes, TODO: use e.g. candy/core/SolverTypes.h instead of core/SolverTypes.h
include_directories(${PROJECT_SOURCE_DIR}/src/candy)

set(GTEST_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib/googletest/googletest/include" "${PROJECT_SOURCE_DIR}/lib/googletest/googlemock/include")

if(CANDY_ENABLE_SONIFICATION)
  # ...at least until we build oscpack via the submodule (TODO):
  include_directories("/usr/local/include")
  include_directories("/usr/include")
endif()

### Add subdirectories (containing CMakeLists with further, directory-specific build instructions)

add_subdirectory(src)

set(CANDY_LIBS frontend gates randomsimulation rsil rsar sonification ipasir core simp utils ${ZLIB_LIBRARIES})

add_subdirectory(lib)
add_subdirectory(testsrc)

### Define and link target executables and libraries

add_executable(candy src/candy/Main.cc)
candy_executable_customizations(candy)
target_link_libraries(candy ${CANDY_LIBS})

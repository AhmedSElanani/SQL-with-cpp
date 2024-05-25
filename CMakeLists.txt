cmake_minimum_required(VERSION 3.22)

# set project name
set(PROJECT_NAME "SQL-with-Cpp-Course" VERSION 1.0.0)

project(${PROJECT_NAME})

# set target compilation options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

# add compilation flags
add_compile_options(
  # stack protection flag
  -fstack-protector-strong
  # necessary warning flags for gcc
  -Wall
  -Wextra
  -Wshadow
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Wcast-align
  -Wunused
  -Woverloaded-virtual
  -Wpedantic
  -Wconversion
  -Wnull-dereference
  -Wformat=2
  -Wmisleading-indentation
  -Wduplicated-cond
  -Wduplicated-branches
  -Wlogical-op
  -Werror
  # diagnostics flags
  -fconcepts-diagnostics-depth=3)

# set additional compile flags for the compiled targets
set(ADDITIONAL_COMPILE_OPTIONS -Wsign-conversion -Wdouble-promotion
                               -Wuseless-cast)

# add link options
add_link_options(-flto)

# enable the generation of the compilation database for all targets
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# option for building unit tests
option(BUILD_TESTS "Option to turn On/OFF building tests" ON)
if(BUILD_TESTS)
  message(STATUS "Building Tests: Enabled")
  add_subdirectory(test)
else()
  message(STATUS "Building Tests: Disabled")
endif()

# option for using ccache
option(USE_CCACHE "Use ccache for compilation" ON)
if(USE_CCACHE)
  # Optionally, set the cache size
  set(CCACHE_MAXSIZE
      "1G"
      CACHE STRING "Maximum size of ccache cache")

  message(STATUS "Using ccache for compilation of max size: " ${CCACHE_MAXSIZE})
  set(CMAKE_C_COMPILER_LAUNCHER "ccache")
  set(CMAKE_CXX_COMPILER_LAUNCHER "ccache")
  set(ENV{CCACHE_MAXSIZE} ${CCACHE_MAXSIZE})
else()
  message(STATUS "Not using ccache for compilation")
endif()

# option for generating Doxygen
option(BUILD_DOCS "Option to turn On/OFF generating Doxygen" OFF)
if(BUILD_DOCS)
  message(STATUS "Generating Doxygen: Enabled")
  add_subdirectory(docs)
else()
  message(STATUS "Generating Doxygen: Disabled")
endif()

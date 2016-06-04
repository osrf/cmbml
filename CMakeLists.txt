cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(cmbml)

# TODO use this in a build type
set(EMBEDDED_OPTIMIZATION_FLAGS "-fno-exceptions -Os -fno-rtti")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_library(cmbml
  src/reader.cpp
  src/writer.cpp
  src/history.cpp
)

target_include_directories(cmbml
  PUBLIC
  include
  # for the future:
  # external/hana/include
  # external/msm-lite/include
)


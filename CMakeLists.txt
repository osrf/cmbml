cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(cmbml)

# TODO use this in a build type
set(EMBEDDED_OPTIMIZATION_FLAGS "-fno-exceptions -Os -fno-rtti")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(external/msm-lite/include/)

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
)

if(ENABLE_TESTING)
  add_executable(cmbml_basic_test test/cmbml.cpp)
  target_link_libraries(cmbml_basic_test ${cmbml})
  target_include_directories(cmbml_basic_test PUBLIC include)
endif()

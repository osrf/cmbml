cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(cmbml)

# TODO use this in a build type
set(EMBEDDED_OPTIMIZATION_FLAGS "-fno-exceptions -Os -fno-rtti")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(external/msm-lite/include/)
include_directories(external/hana/include/)

add_library(cmbml
  STATIC
  src/common.cpp
  src/dds/condition.cpp
  src/history.cpp
  src/reader.cpp
  src/writer.cpp
  src/psm/udp/context.cpp
)

target_include_directories(cmbml
  PUBLIC
  include
)

function(basic_cmbml_test test_name src)
  add_executable(${test_name} ${src})
  target_link_libraries(${test_name} cmbml)
  target_include_directories(${test_name} PUBLIC include)
endfunction()

if(ENABLE_TESTING)
  # basic_cmbml_test(cmbml_test test/cmbml.cpp)

  basic_cmbml_test(serialization_test test/serialization.cpp)

  basic_cmbml_test(nominal_usage test/nominal_usage.cpp)
endif()

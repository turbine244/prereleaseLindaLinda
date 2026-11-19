cmake_minimum_required(VERSION 3.16)

project(lindaAsio LANGUAGES CXX)

get_filename_component(LINDA_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

add_library(lindaAsio SHARED
  src/dllExport.cpp
  src/_h/01_api/lindaAsio.cpp
  src/_h/00_runtime/lindaAsioRuntime.cpp
  src/_h/01_api/narrative/narr.openAudioRuntime.cpp
  src/_h/01_api/narrative/narr.scanChannel.cpp

  ${LINDA_ROOT}/00_vendored/asio_sdk/common/asio.cpp
  ${LINDA_ROOT}/00_vendored/asio_sdk/host/asiodrivers.cpp
  ${LINDA_ROOT}/00_vendored/asio_sdk/host/pc/asiolist.cpp
)

set_target_properties(lindaAsio PROPERTIES
  OUTPUT_NAME "linda.asio"
)

target_include_directories(lindaAsio PRIVATE
  ${LINDA_ROOT}
  ${LINDA_ROOT}/00_vendored/asio_sdk/common
  ${LINDA_ROOT}/00_vendored/asio_sdk/host
  ${LINDA_ROOT}/00_vendored/asio_sdk/host/pc
)

target_compile_features(lindaAsio PRIVATE cxx_std_20)
target_compile_definitions(lindaAsio PRIVATE LINDA_SUPPORT_BUILD)

if(MSVC)
  target_compile_options(lindaAsio PRIVATE /utf-8)
endif()

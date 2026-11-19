cmake_minimum_required(VERSION 3.16)

project(lindaJs LANGUAGES C CXX)

get_filename_component(LINDA_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

add_subdirectory(
  "${LINDA_ROOT}/00_vendored/quickjs-ng"
  "${CMAKE_CURRENT_BINARY_DIR}/quickjs-ng"
  EXCLUDE_FROM_ALL
)

set_target_properties(qjs PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(lindaJs SHARED
  src/dllExport.cpp
  src/_h/00_file/lindaJsFile.cpp
  src/_h/01_runtime/lindaJsRuntime.cpp
  src/_h/02_api/narrative/narr.scanSemantic.cpp
  src/_h/02_api/narrative/narr.buildRig.cpp
  src/_h/02_api/narrative/narr.openLanguageRuntime.cpp
  src/_h/02_api/lindaJs.cpp

  ${LINDA_ROOT}/00_vendored/tree-sitter/lib/src/lib.c
  ${LINDA_ROOT}/00_vendored/tree-sitter-javascript/src/parser.c
  ${LINDA_ROOT}/00_vendored/tree-sitter-javascript/src/scanner.c
)

set_target_properties(lindaJs PROPERTIES
  OUTPUT_NAME "linda.js"
)

target_include_directories(lindaJs PRIVATE
  ${LINDA_ROOT}
  ${LINDA_ROOT}/00_vendored/tree-sitter/lib/include
  ${LINDA_ROOT}/00_vendored/tree-sitter-javascript/src
)

target_compile_features(lindaJs PRIVATE c_std_11 cxx_std_20)
target_compile_definitions(lindaJs PRIVATE LINDA_SUPPORT_BUILD)
target_link_libraries(lindaJs PRIVATE qjs)

if(MSVC)
  target_compile_options(lindaJs PRIVATE /utf-8)
endif()

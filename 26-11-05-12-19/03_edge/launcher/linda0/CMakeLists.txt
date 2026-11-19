cmake_minimum_required(VERSION 3.16)

project(linda0 LANGUAGES CXX)

get_filename_component(LINDA_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

add_executable(linda0
  src/main.cpp
  src/_h/00_file/00_rig/rigPackage.cpp
  src/_h/01_runtime/00_audioSupport/audioSupport.cpp
  src/_h/01_runtime/01_languageSupport/languageSupport.cpp
  src/_h/01_runtime/02_gig/gig.cpp
  src/_h/01_runtime/02_gig/narrative/narr.openGigRuntime.cpp
  src/_h/02_shell/00_cli/cli.cpp
  src/_h/02_shell/01_command/support.cpp
  src/_h/02_shell/01_command/list.cpp
  src/_h/02_shell/01_command/scan.cpp
  src/_h/02_shell/01_command/build.cpp
  src/_h/02_shell/01_command/narrative/narr.commandBuild.cpp
  src/_h/02_shell/01_command/run.cpp
  src/_h/02_shell/01_command/narrative/narr.commandRun.cpp
  src/_h/02_shell/02_dispatch/dispatch.cpp
)

target_include_directories(linda0 PRIVATE
  ${LINDA_ROOT}
)

target_compile_features(linda0 PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(linda0 PRIVATE /utf-8)
endif()



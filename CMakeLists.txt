cmake_minimum_required(VERSION 3.24)
project(euchre LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE EUCHRE_SRC CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

list(REMOVE_ITEM EUCHRE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

option(ENABLE_SANITIZERS "Enable sanitizers" ON)

add_library(sanitizers INTERFACE)

if (ENABLE_SANITIZERS AND NOT MSVC)
  target_compile_options(sanitizers INTERFACE
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
    -g
  )
  target_link_options(sanitizers INTERFACE
    -fsanitize=address,undefined
  )
endif()

# Euchre Library
add_library(euchre_lib)
target_sources(euchre_lib PRIVATE ${EUCHRE_SRC})
target_include_directories(euchre_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(euchre_lib PRIVATE sanitizers)

# Euchre Main executable
add_executable(euchre src/main.cpp)
#target_include_directories(euchre PRIVATE include)
target_link_libraries(euchre PRIVATE euchre_lib sanitizers)

# Catch2 
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.12.0   # pin a version for reproducible builds
)
FetchContent_MakeAvailable(Catch2)

# Tests
include(CTest)
if (BUILD_TESTING)
  add_executable(tests
    tests/test_cards.cpp
    tests/test_tables.cpp
    tests/test_deck.cpp
  )
  target_link_libraries(tests PRIVATE euchre_lib Catch2::Catch2WithMain)

  # Nice: auto-registers each TEST_CASE with ctest
  include(Catch)
  catch_discover_tests(tests)
endif()

function(enable_warnings_as_errors target)
  if (MSVC)
    target_compile_options(${target} PRIVATE /WX)
  else()
    target_compile_options(${target} PRIVATE -Werror)
  endif()
endfunction()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  enable_warnings_as_errors(euchre_lib)
endif()

function(enable_warnings target)
  if (MSVC)
    target_compile_options(${target} PRIVATE
      /W4            # High warning level
      /permissive-   # Strict standard conformance
      /w14242        # conversion, sign issues
      /w14254
      /w14263
      /w14265
      /w14287
      /we4289        # treat specific warnings as errors
    )
  else()
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wshadow
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Woverloaded-virtual
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
    )
  endif()
endfunction()

enable_warnings(euchre_lib)
enable_warnings(euchre)
if (BUILD_TESTING)
  enable_warnings(tests)
endif()
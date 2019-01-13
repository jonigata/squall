cmake_minimum_required (VERSION 3.11)
project(squall)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

option(BUILD_EXAMPLES "Build the examples" ON)

find_package(squirrel QUIET)
if (NOT squirrel_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    squirrel
    GIT_REPOSITORY https://github.com/albertodemichelis/squirrel.git
    GIT_TAG 780e9c96b34d24d347c90756efa96f2927fcb641
  )
  FetchContent_GetProperties(squirrel)
  if (NOT squirrel_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(squirrel)
    add_subdirectory(${squirrel_SOURCE_DIR} ${squirrel_BINARY_DIR})
  endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${squirrel_SOURCE_DIR}/include)

add_subdirectory(samples)

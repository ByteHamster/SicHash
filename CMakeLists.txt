cmake_minimum_required(VERSION 3.25)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(SicHash)

if(TARGET SicHash)
    return()
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND PROJECT_IS_TOP_LEVEL)
    add_compile_options(-march=native)
endif()

set(TLX_INSTALL_INCLUDE_DIR tlx CACHE PATH "Workaround for TLX breaking the first cmake call")
add_subdirectory(extlib/tlx)

add_library(SicHash INTERFACE)
target_include_directories(SicHash INTERFACE include)
target_compile_features(SicHash INTERFACE cxx_std_20)

add_subdirectory(extlib/util EXCLUDE_FROM_ALL)
target_link_libraries(SicHash INTERFACE ByteHamster::Util)

add_subdirectory(extlib/simpleRibbon EXCLUDE_FROM_ALL)
target_link_libraries(SicHash INTERFACE SimpleRibbon ips2ra)

add_library(SicHash::sichash ALIAS SicHash)

if(PROJECT_IS_TOP_LEVEL)
    target_compile_options(SicHash INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic -Werror -frecord-gcc-switches>)

    add_executable(Example src/example.cpp)
    target_link_libraries(Example PRIVATE SicHash)
    target_compile_features(Example PRIVATE cxx_std_20)

    add_executable(Solvers src/solvers.cpp)
    target_link_libraries(Solvers PRIVATE SicHash tlx)
    target_compile_features(Solvers PRIVATE cxx_std_20)

    add_executable(ConstructionSuccess src/constructionSuccess.cpp)
    target_link_libraries(ConstructionSuccess PRIVATE SicHash tlx)
    target_compile_features(ConstructionSuccess PRIVATE cxx_std_20)

    add_executable(SicHashBenchmark src/sicHashBenchmark.cpp)
    target_link_libraries(SicHashBenchmark PRIVATE SicHash tlx)
    target_compile_features(SicHashBenchmark PRIVATE cxx_std_20)

    add_executable(MaxLoadFactor src/maxLoadFactor.cpp)
    target_link_libraries(MaxLoadFactor PRIVATE SicHash tlx)
    target_compile_features(MaxLoadFactor PRIVATE cxx_std_20)
endif()

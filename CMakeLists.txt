cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(SicHash)

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_compile_options(-march=native)
endif()

set(TLX_INSTALL_INCLUDE_DIR tlx CACHE PATH "Workaround for TLX breaking the first cmake call")
add_subdirectory(extlib/tlx)

add_library(SicHash INTERFACE)
target_include_directories(SicHash INTERFACE include)
target_compile_features(SicHash INTERFACE cxx_std_20)

add_subdirectory(extlib/util EXCLUDE_FROM_ALL)
target_link_libraries(SicHash INTERFACE ByteHamsterUtil)

add_subdirectory(extlib/simpleRibbon)
target_link_libraries(SicHash INTERFACE SimpleRibbon)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    ################### Benchmark build targets ###################
    add_executable(Solvers solvers.cpp)
    target_link_libraries(Solvers PRIVATE SicHash)
    target_compile_features(Solvers PRIVATE cxx_std_20)

    add_executable(ConstructionSuccess constructionSuccess.cpp)
    target_link_libraries(ConstructionSuccess PRIVATE SicHash)
    target_compile_features(ConstructionSuccess PRIVATE cxx_std_20)

    add_executable(SicHashBenchmark sicHashBenchmark.cpp)
    target_link_libraries(SicHashBenchmark PRIVATE SicHash)
    target_compile_features(SicHashBenchmark PRIVATE cxx_std_20)

    add_executable(MaxLoadFactor maxLoadFactor.cpp)
    target_link_libraries(MaxLoadFactor PRIVATE SicHash)
    target_compile_features(MaxLoadFactor PRIVATE cxx_std_20)
endif()

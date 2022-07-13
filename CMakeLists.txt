cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(SicHash)

add_library(SicHash INTERFACE)
target_include_directories(SicHash INTERFACE include)
target_compile_features(SicHash INTERFACE cxx_std_20)

add_subdirectory(extlib/util EXCLUDE_FROM_ALL)
target_link_libraries(SicHash INTERFACE ByteHamsterUtil)

set(TLX_INSTALL_INCLUDE_DIR tlx CACHE PATH "Workaround for TLX breaking the first cmake call")
set(IPS2RA_DISABLE_PARALLEL ON CACHE PATH "FindTBB greps a file that does not exist in recent TBB versions")
add_subdirectory(extlib/ribbon/ips2ra)
add_subdirectory(extlib/ribbon/tlx)
add_library(Ribbon SHARED include/SimpleRibbon.cpp)
target_include_directories(Ribbon PRIVATE extlib/ribbon/DySECT)
target_include_directories(Ribbon PRIVATE extlib/ribbon/ips2ra/include)
target_include_directories(Ribbon PUBLIC extlib/ribbon/tlx)
target_include_directories(Ribbon PUBLIC extlib/ribbon)
target_link_libraries(Ribbon PRIVATE tlx ips2ra)
target_link_libraries(SicHash INTERFACE Ribbon)

add_library(RibbonSorter SHARED extlib/ribbon/sorter.cpp)
target_include_directories(RibbonSorter PRIVATE extlib/ribbon/ips2ra/include)
target_link_libraries(RibbonSorter PUBLIC tlx ips2ra)
target_link_libraries(SicHash INTERFACE RibbonSorter)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    ###################### Competitor setup ######################
    add_library(PerfectHashingCompetitors INTERFACE)
    target_compile_features(PerfectHashingCompetitors INTERFACE cxx_std_20)
    target_include_directories(PerfectHashingCompetitors SYSTEM INTERFACE extlib/pthash/include)
    target_include_directories(PerfectHashingCompetitors SYSTEM INTERFACE extlib/BBHash)
    target_link_libraries(PerfectHashingCompetitors INTERFACE SicHash)

    file(GLOB cmphSources extlib/cmph/src/**.c)
    list(FILTER cmphSources EXCLUDE REGEX ".*cmph/src/(bdz_gen_lookup_table|bm_numbers|sdbm_hash|fnv_hash|djb2_hash|hashtree|buffer_manage|main).c$")
    add_library(Cmph SHARED ${cmphSources})
    target_include_directories(Cmph SYSTEM INTERFACE extlib/cmph/src)
    target_link_libraries(PerfectHashingCompetitors INTERFACE Cmph)

    add_library(Sux INTERFACE)
    target_include_directories(Sux SYSTEM INTERFACE extlib/sux)
    target_link_libraries(PerfectHashingCompetitors INTERFACE Sux)

    ################### Benchmark build targets ###################
    add_executable(Comparison comparison.cpp)
    target_link_libraries(Comparison PRIVATE PerfectHashingCompetitors)
    target_compile_features(Comparison PRIVATE cxx_std_20)

    add_executable(ComparisonN comparisonN.cpp)
    target_link_libraries(ComparisonN PRIVATE PerfectHashingCompetitors)
    target_compile_features(ComparisonN PRIVATE cxx_std_20)

    add_executable(Solvers solvers.cpp)
    target_link_libraries(Solvers PRIVATE SicHash)
    target_compile_features(Solvers PRIVATE cxx_std_20)

    add_executable(ConstructionSuccess constructionSuccess.cpp)
    target_link_libraries(ConstructionSuccess PRIVATE SicHash)
    target_compile_features(ConstructionSuccess PRIVATE cxx_std_20)

    add_executable(PhfBenchmark phfBenchmark.cpp)
    target_link_libraries(PhfBenchmark PRIVATE SicHash)
    target_compile_features(PhfBenchmark PRIVATE cxx_std_20)

    add_executable(PhfBenchmarkSingle phfBenchmarkSingle.cpp)
    target_link_libraries(PhfBenchmarkSingle PRIVATE SicHash)
    target_compile_features(PhfBenchmarkSingle PRIVATE cxx_std_20)

    add_executable(UnitTest unitTest.cpp)
    target_link_libraries(UnitTest PRIVATE SicHash)
    target_compile_features(UnitTest PRIVATE cxx_std_20)

    add_executable(MaxLoadFactor maxLoadFactor.cpp)
    target_link_libraries(MaxLoadFactor PRIVATE SicHash)
    target_compile_features(MaxLoadFactor PRIVATE cxx_std_20)
endif()

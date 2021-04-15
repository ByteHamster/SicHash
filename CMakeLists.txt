cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(PerfectHashingComparison)

file(GLOB sources hashAssignment/**.cpp util/**.cpp hdc/**.cpp variableSize/**.cpp)
list(FILTER sources EXCLUDE REGEX ".*util/SimpleRibbon.cpp$")
add_library(PerfectHashingMethods ${sources})
target_compile_features(PerfectHashingMethods PRIVATE cxx_std_20)

add_executable(Comparison comparison.cpp)
target_link_libraries(Comparison PRIVATE PerfectHashingMethods)
target_compile_features(Comparison PRIVATE cxx_std_20)

add_executable(OldPlots old_plots.cpp)
target_link_libraries(OldPlots PRIVATE PerfectHashingMethods)
target_compile_features(OldPlots PRIVATE cxx_std_20)

add_executable(Plots plots.cpp)
target_link_libraries(Plots PRIVATE PerfectHashingMethods)
target_compile_features(Plots PRIVATE cxx_std_20)

add_library(Sux INTERFACE)
target_include_directories(Sux INTERFACE extlib/sux)
target_link_libraries(PerfectHashingMethods PUBLIC Sux)

file(GLOB cmphSources extlib/cmph/src/**.c)
list(FILTER cmphSources EXCLUDE REGEX ".*cmph/src/(bdz_gen_lookup_table|bm_numbers|sdbm_hash|fnv_hash|djb2_hash|hashtree|buffer_manage|main).c$")
add_library(Cmph SHARED ${cmphSources})
target_include_directories(Cmph PUBLIC extlib/cmph/src)
target_link_libraries(PerfectHashingMethods PUBLIC Cmph)

add_library(Matching SHARED extlib/weighted-bipartite-perfect-matching/hungarian.cpp)
target_include_directories(Matching PUBLIC extlib/weighted-bipartite-perfect-matching)
target_link_libraries(PerfectHashingMethods PUBLIC Matching)

set(TLX_INSTALL_INCLUDE_DIR tlx CACHE PATH "Workaround for TLX breaking the first cmake call")
set(IPS2RA_DISABLE_PARALLEL ON CACHE PATH "FindTBB greps a file that does not exist in recent TBB versions")
add_subdirectory(extlib/ribbon/ips2ra)
add_subdirectory(extlib/ribbon/tlx)
add_library(Ribbon SHARED util/SimpleRibbon.cpp)
target_include_directories(Ribbon PRIVATE extlib/ribbon/DySECT)
target_include_directories(Ribbon PRIVATE extlib/ribbon/ips2ra/include)
target_include_directories(Ribbon PRIVATE extlib/ribbon/tlx)
target_link_libraries(Ribbon PRIVATE tlx ips2ra)
target_link_libraries(PerfectHashingMethods PUBLIC Ribbon)

add_library(RibbonSorter SHARED extlib/ribbon/sorter.cpp)
target_include_directories(RibbonSorter PRIVATE extlib/ribbon/ips2ra/include)
target_link_libraries(RibbonSorter PRIVATE tlx ips2ra)
target_link_libraries(PerfectHashingMethods PUBLIC RibbonSorter)

find_package(OpenMP REQUIRED)
target_link_libraries(PerfectHashingMethods PUBLIC pthread)


cmake_minimum_required(VERSION 3.10)

# Set the project name
project(DataflowSim)

set (CMAKE_BUILD_TYPE "Debug")
# set (CMAKE_BUILD_TYPE "Release")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_compile_options()

file(GLOB_RECURSE SRC_HEADER ${PROJECT_SOURCE_DIR}/DataflowSim/src/*.h)
file(GLOB_RECURSE SRC_HPP ${PROJECT_SOURCE_DIR}/DataflowSim/src/*.hpp)
file(GLOB_RECURSE SRC_CPP ${PROJECT_SOURCE_DIR}/DataflowSim/src/*.cpp)
file(GLOB_RECURSE TEST_HEADER ${PROJECT_SOURCE_DIR}/DataflowSim/test/*.h)
file(GLOB_RECURSE TEST_HPP ${PROJECT_SOURCE_DIR}/DataflowSim/test/*.hpp)
file(GLOB_RECURSE TEST_CPP ${PROJECT_SOURCE_DIR}/DataflowSim/test/*.cpp)

# aux_source_directory (${PROJECT_SOURCE_DIR}/src SRC)
# aux_source_directory (${PROJECT_SOURCE_DIR}/src/define DEFINE)
# aux_source_directory (${PROJECT_SOURCE_DIR}/src/module/ MODULE)
# aux_source_directory (${PROJECT_SOURCE_DIR}/src/module/execution EXECUTION)
# aux_source_directory (${PROJECT_SOURCE_DIR}/src/module/mem MEMORY)
# aux_source_directory (${PROJECT_SOURCE_DIR}/src/sim SIM)
# aux_source_directory (${PROJECT_SOURCE_DIR}/src/util UTIL)
# aux_source_directory (${PROJECT_SOURCE_DIR}/test/gcn GCN)
# aux_source_directory (${PROJECT_SOURCE_DIR}/test/gemm GEMM)
# aux_source_directory (${PROJECT_SOURCE_DIR}/test/. TEST)
aux_source_directory (${PROJECT_SOURCE_DIR}/DataflowSim/lib/dramsim3/lib DRAMSIM3LIB)
aux_source_directory (${PROJECT_SOURCE_DIR}/DataflowSim/lib/jsoncpp/lib JSONCPPLIB)

# include_directories (
# ${PROJECT_SOURCE_DIR}/src
# ${PROJECT_SOURCE_DIR}/src/define/Para.h
# ${PROJECT_SOURCE_DIR}/test 
# ${PROJECT_SOURCE_DIR}/DRAMSim3/include
# )

include_directories (
SRC_HEADER
SRC_HPP
SRC_CPP
TEST_HEADER
TEST_HPP
TEST_CPP
${PROJECT_SOURCE_DIR}/DataflowSim/lib/dramsim3/include
${PROJECT_SOURCE_DIR}/DataflowSim/lib/jsoncpp/include
)

find_library(DRAMSIM3_LIB dramsim3 HINTS ${PROJECT_SOURCE_DIR}/DataflowSim/lib/dramsim3/lib)
find_library(JSONCPP_LIB jsoncpp HINTS ${PROJECT_SOURCE_DIR}/DataflowSim/lib/jsoncpp/lib)

# add_executable (
# dfsim_citeseer
# ${SRC}
# ${DEFINE} 
# ${MODULE} 
# ${EXECUTION} 
# ${MEMORY} 
# ${SIM} 
# ${UTIL} 
# ${GCN}
# ${GEMM}
# ${TEST} 
# ${DRAMSIM3LIB}
# )

add_executable (
dfsim
${SRC_HEADER}
${SRC_HPP}
${SRC_CPP}
${TEST_HEADER}
${TEST_HPP}
${TEST_CPP}
${DRAMSIM3LIB}
${JSONCPPLIB}
)

target_link_libraries (dfsim ${DRAMSIM3_LIB} ${JSONCPP_LIB})



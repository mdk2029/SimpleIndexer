cmake_minimum_required(VERSION 3.1)

##############################################################
# Some common stuff
##############################################################
option(USE_GCC "Use gcc instead of clang" OFF)

if(USE_GCC)
    set(CMAKE_CXX_COMPILER g++ CACHE STRING "CXX Compiler")
    set(CMAKE_C_COMPILER gcc CACHE STRING "C Compiler")
else()
    set(CMAKE_CXX_COMPILER clang++ CACHE STRING "CXX Compiler")
    set(CMAKE_C_COMPILER clang CACHE STRING "C Compiler")
endif()

set(CMAKE_CXX_STANDARD 14 CACHE STRING "CXX Standard")
set(CMAKE_VERBOSE_MAKEFILE true CACHE BOOL "verbose make output")
set(THREADS_PREFER_PTHREAD_FLAG ON CACHE STRING "use pthread")

##############################################################
# Define the project
##############################################################

project(wordIndexer LANGUAGES CXX) 

# Use -O3 instead of -O2
string(REPLACE "-O2" "-O3" newFlags ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELEASE "${newFlags}")
message("CMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}")

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)
find_package(gtest REQUIRED HINTS ${CMAKE_CURRENT_LIST_DIR}/cmake)

file(GLOB SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp" 
                  "${CMAKE_CURRENT_LIST_DIR}/*.h" 
                  "${CMAKE_CURRENT_LIST_DIR}/tests/Tests.cpp"
) 

message("SOURCES=${SOURCES}")
add_executable(${PROJECT_NAME} "")
target_sources(${PROJECT_NAME}
        PUBLIC
        "${SOURCES}"
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/tests
    )
target_link_libraries(${PROJECT_NAME}
    boost_filesystem
    boost_system
    boost_program_options
    gtest
    Threads::Threads)

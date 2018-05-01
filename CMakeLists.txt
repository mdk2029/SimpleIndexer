cmake_minimum_required(VERSION 3.1)
include($ENV{HOME}/GlobalCMakeLists.txt)

project(wordIndexer LANGUAGES CXX) 
message("project name: ${PROJECT_NAME}")

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

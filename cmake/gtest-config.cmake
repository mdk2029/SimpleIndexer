# Download and unpack googletest at configure time

configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeListsGoogleTest.txt.in
    ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
                 "${CMAKE_BINARY_DIR}/googletest-build")

target_include_directories(gmock_main SYSTEM BEFORE INTERFACE
    "${gtest_SOURCE_DIR}/include" "${gmock_SOURCE_DIR}/include")

set(GTEST_FOUND true CACHE BOOL "gtest found")
set(GTEST_INCLUDE_DIR "${gtest_SOURCE_DIR}/include" CACHE FILEPATH "gtest include dir")
set(GTEST_LIBRARY "gtest" CACHE STRING "gtest lib name")
set(GTEST_MAIN_LIBRARY "gtest_main" CACHE STRING "gtest_main lib name")

set(GMOCK_FOUND true CACHE BOOL "gmock found")
set(GMOCK_INCLUDE_DIR "${gmock_SOURCE_DIR}/include" CACHE FILEPATH "gmock include dir")
set(GMOCK_LIBRARY "gmock" CACHE STRING "gmock lib name")
set(GMOCK_MAIN_LIBRARY "gmock_main" CACHE STRING "gmock_main lib name")

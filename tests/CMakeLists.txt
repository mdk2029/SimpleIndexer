
# Download and unpack googletest at configure time

find_package(gtest REQUIRED HINTS ${CMAKE_CURRENT_LIST_DIR}/../cmake)

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
add_executable(wordIndexer_test "")
target_sources(wordIndexer_test PUBLIC
    "${TEST_SOURCES}"
    )
target_link_libraries(wordIndexer_test ${PROJECT_NAME} gmock_main )
target_include_directories(wordIndexer_test
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )


include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
message(STATUS "  Fetching Google Test...")
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")
list(LENGTH TEST_SOURCES TEST_SOURCES_COUNT)
message(STATUS "  Test sources: ${TEST_SOURCES_COUNT} files")

add_executable(serialkit_tests ${TEST_SOURCES})
target_link_libraries(serialkit_tests PRIVATE 
    serialkit_compiler_lib 
    GTest::gtest_main
)

add_test(NAME serialkit_tests COMMAND serialkit_tests)

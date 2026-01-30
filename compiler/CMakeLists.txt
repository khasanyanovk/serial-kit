file(GLOB COMPILER_SOURCES "source/*.cpp")
file(GLOB COMPILER_HEADERS "include/*.hpp")
list(REMOVE_ITEM COMPILER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp")

list(LENGTH COMPILER_SOURCES COMPILER_SOURCES_COUNT)
message(STATUS "  Compiler sources: ${COMPILER_SOURCES_COUNT} files")

add_library(serialkit_compiler_lib STATIC ${COMPILER_SOURCES} ${COMPILER_HEADERS})

target_include_directories(serialkit_compiler_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(serialkit_compiler_lib PUBLIC SerialKit::Runtime)

add_executable(serialkit_compiler main.cpp)
target_link_libraries(serialkit_compiler PRIVATE serialkit_compiler_lib)
set_target_properties(serialkit_compiler PROPERTIES OUTPUT_NAME "serialkit-compiler")

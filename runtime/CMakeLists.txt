file(GLOB RUNTIME_SOURCES "source/*.cpp")
file(GLOB RUNTIME_HEADERS "include/*.hpp")

list(LENGTH RUNTIME_SOURCES RUNTIME_SOURCES_COUNT)
message(STATUS "  Runtime sources: ${RUNTIME_SOURCES_COUNT} files")

add_library(serialkit_runtime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS})

add_library(SerialKit::Runtime ALIAS serialkit_runtime)

target_include_directories(serialkit_runtime PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(serialkit_runtime PRIVATE Threads::Threads)
